import argparse
import os
import sys

from PIL import Image


def main():
    parser = argparse.ArgumentParser(description="Shrink cover images for the EmulationStation port.")
    parser.add_argument("source", help="folder with the original covers (PNG)")
    parser.add_argument("output", help="folder for the resized covers")
    parser.add_argument("--width", type=int, default=512, help="output width in pixels")
    args = parser.parse_args()

    os.makedirs(args.output, exist_ok=True)
    converted = 0
    for name in sorted(os.listdir(args.source)):
        if not name.lower().endswith(".png"):
            continue
        target = os.path.join(args.output, name)
        if os.path.exists(target):
            continue
        with Image.open(os.path.join(args.source, name)) as image:
            if image.width > args.width:
                image = image.resize((args.width, round(image.height * args.width / image.width)), Image.LANCZOS)
            image.save(target, "PNG", optimize=True)
        converted += 1
        sys.stdout.write("\r%d converted" % converted)
        sys.stdout.flush()
    sys.stdout.write("\n")


if __name__ == "__main__":
    main()
