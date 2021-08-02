from PIL import Image
import os
import sys


def convertImage(img, output_path):
    try:
        with open(os.path.splitext(output_path)[0] + ".h", 'w') as f:
            f.write(
                f"const u32 gImage_{name}[{img.width*img.height*4}] = ")
            f.write("{\n")

            count = 0
            for y in range(img.height):
                for x in range(img.width):
                    color = img.getpixel((x, y))
                    value = color[2] + (color[1] << 8) + (color[0] << 16)
                    f.write(f"{value:#08X},")
                    count += 1
                    if(count % 12 == 0):
                        f.write('\n')
            f.write("};\n")
    except PermissionError:
        print(f"Permission denied: {os.path.splitext(output_path)[0] + '.h'}")


def removesuffix(s, suffix):
    if suffix and s.endswith(suffix):
        return s[:-len(suffix)]
    return s


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage : python imageToArr.py [input image] [output file]")
        sys.exit(-1)
    img_path = os.path.abspath(sys.argv[1])
    output_path = os.path.abspath(sys.argv[2])

    if not os.path.isfile(img_path):
        print(img_path + " does not exist")
        sys.exit(-1)

    name = removesuffix(os.path.basename(output_path), ".h")
    if not ("___"+name).isidentifier():
        print(f"invalid variable name: {name}")
        sys.exit(-1)

    try:
        img = Image.open(img_path).convert("RGB")
    except OSError:
        print(img_path + " is not an image file")
        sys.exit(-1)
    else:
        convertImage(img, output_path)
