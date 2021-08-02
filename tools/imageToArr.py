from PIL import Image

NAME = "deej_logo"

img = Image.open("logo_black.png").convert("RGB")
with open(NAME+".h", 'w') as f:
    f.write(
        f"__align(4) const unsigned char gImage_{NAME}[{img.width*img.height*4}] = ")
    f.write("{\n")

    count = 0
    pixel_values = ""
    for y in range(img.height):
        for x in range(img.width):
            rgb = img.getpixel((x, y))
            for value in reversed(rgb):
                count += 1
                f.write((hex(value).upper()) + ',')
            count += 1
            f.write("0X00,")
            if(count % 16 == 0):
                f.write('\n')
    f.write("};\n")
