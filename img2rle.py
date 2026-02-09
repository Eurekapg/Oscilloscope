"""
Convert image to palette-indexed RLE-compressed data for ST7735 TFT (160x128).

Strategy:
1. Resize image to fit 160x128
2. Quantize to N colors (palette)
3. Store palette as RGB565 (N * 2 bytes)
4. RLE compress palette indices (1 byte per pixel -> great runs)
   Format: [count(u8)] [index(u8)] = 2 bytes per run

Much better compression than raw RGB565 RLE for detailed images.
"""

from PIL import Image
import sys
import os

def rgb888_to_rgb565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

def try_palette_rle(canvas, num_colors):
    """Try a specific palette size and return (compressed_size, palette_data, rle_data)."""
    # Quantize
    img_q = canvas.quantize(colors=num_colors, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.FLOYDSTEINBERG)
    palette_rgb = img_q.getpalette()  # flat list: [r0,g0,b0, r1,g1,b1, ...]
    actual_colors = img_q.getextrema()[1] + 1  # max index + 1

    # Build palette in RGB565
    palette_565 = []
    for i in range(actual_colors):
        r, g, b = palette_rgb[i*3], palette_rgb[i*3+1], palette_rgb[i*3+2]
        palette_565.append(rgb888_to_rgb565(r, g, b))

    # Get pixel indices
    indices = list(img_q.getdata())

    # RLE compress indices
    rle = bytearray()
    i = 0
    while i < len(indices):
        cur = indices[i]
        count = 1
        while i + count < len(indices) and indices[i + count] == cur and count < 255:
            count += 1
        rle.append(count)
        rle.append(cur)
        i += count

    # Palette bytes
    pal_bytes = bytearray()
    for c in palette_565:
        pal_bytes.append((c >> 8) & 0xFF)
        pal_bytes.append(c & 0xFF)

    total = len(pal_bytes) + len(rle)
    return total, pal_bytes, rle, actual_colors

def main():
    img_path = sys.argv[1] if len(sys.argv) > 1 else None

    if not img_path:
        base = os.path.dirname(os.path.abspath(__file__))
        for f in os.listdir(base):
            if f.lower().endswith(('.jpg', '.jpeg', '.png', '.bmp')):
                img_path = os.path.join(base, f)
                break

    if not img_path or not os.path.exists(img_path):
        print("No image file found!")
        return

    print(f"Input: {img_path}")
    img = Image.open(img_path).convert('RGB')
    print(f"Original: {img.width}x{img.height}")

    target_w, target_h = 160, 128
    scale = min(target_w / img.width, target_h / img.height)
    new_w = int(img.width * scale)
    new_h = int(img.height * scale)

    img_resized = img.resize((new_w, new_h), Image.LANCZOS)
    canvas = Image.new('RGB', (target_w, target_h), (0, 0, 0))
    offset_x = (target_w - new_w) // 2
    offset_y = (target_h - new_h) // 2
    canvas.paste(img_resized, (offset_x, offset_y))

    print(f"Resized: {new_w}x{new_h}, Canvas: {target_w}x{target_h}")
    raw_size = target_w * target_h * 2
    print(f"Raw RGB565: {raw_size} bytes\n")

    # Try different palette sizes
    best = None
    for nc in [32, 48, 64, 96, 128]:
        total, pal_bytes, rle, actual = try_palette_rle(canvas, nc)
        ratio = raw_size / total
        print(f"  {nc:3d} colors -> {actual:3d} actual, palette={len(pal_bytes)}B, "
              f"rle={len(rle)}B, total={total}B ({total/1024:.1f}KB), ratio={ratio:.2f}:1")
        if best is None or total < best[0]:
            best = (total, pal_bytes, rle, actual, nc)

    # Check if best fits in flash (aim for <25KB to leave room for code)
    total, pal_bytes, rle_data, actual_colors, chosen_nc = best
    if total > 25000:
        # Try smaller palette
        for nc in [32, 24, 20, 16]:
            t, pb, rd, ac = try_palette_rle(canvas, nc)
            print(f"  {nc:3d} colors -> {ac:3d} actual, total={t}B ({t/1024:.1f}KB)")
            if t < total:
                total, pal_bytes, rle_data, actual_colors, chosen_nc = t, pb, rd, ac, nc
                if t <= 22000:
                    break

    print(f"\nChosen: {chosen_nc} colors ({actual_colors} actual)")
    print(f"Palette: {len(pal_bytes)} bytes")
    print(f"RLE data: {len(rle_data)} bytes")
    print(f"Total: {total} bytes ({total/1024:.1f} KB)")
    print(f"Compression: {raw_size/total:.2f}:1")

    # Generate C header
    out_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "APP")
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, "kanade_img.h")

    with open(out_path, 'w') as f:
        f.write("#ifndef __KANADE_IMG_H\n")
        f.write("#define __KANADE_IMG_H\n\n")
        f.write("#include <stdint.h>\n\n")
        f.write(f"/* Auto-generated from: {os.path.basename(img_path)} */\n")
        f.write(f"/* {target_w}x{target_h}, {actual_colors}-color palette, RLE indexed */\n")
        f.write(f"/* Palette: {len(pal_bytes)}B + RLE: {len(rle_data)}B = {total}B total */\n")
        f.write(f"/* Format: palette[N] as RGB565, rle as [count(u8)][index(u8)] pairs */\n\n")
        f.write(f"#define KANADE_IMG_W {target_w}\n")
        f.write(f"#define KANADE_IMG_H {target_h}\n")
        f.write(f"#define KANADE_PAL_SIZE {actual_colors}\n")
        f.write(f"#define KANADE_RLE_SIZE {len(rle_data)}\n\n")

        # Palette array (RGB565)
        f.write(f"static const uint16_t kanade_palette[{actual_colors}] = {{\n")
        for i in range(0, actual_colors, 8):
            chunk = []
            for j in range(i, min(i+8, actual_colors)):
                hi = pal_bytes[j*2]
                lo = pal_bytes[j*2+1]
                val = (hi << 8) | lo
                chunk.append(f"0x{val:04X}")
            line = ", ".join(chunk)
            if i + 8 < actual_colors:
                f.write(f"    {line},\n")
            else:
                f.write(f"    {line}\n")
        f.write("};\n\n")

        # RLE data
        f.write(f"static const uint8_t kanade_rle_data[{len(rle_data)}] = {{\n")
        for i in range(0, len(rle_data), 24):
            chunk = rle_data[i:i+24]
            line = ", ".join(f"0x{b:02X}" for b in chunk)
            if i + 24 < len(rle_data):
                f.write(f"    {line},\n")
            else:
                f.write(f"    {line}\n")
        f.write("};\n\n")

        f.write("#endif /* __KANADE_IMG_H */\n")

    print(f"\nOutput: {out_path}")

if __name__ == "__main__":
    main()
