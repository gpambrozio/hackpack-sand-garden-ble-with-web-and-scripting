#!/usr/bin/env python3
"""
Generate icon.png and logo.png files for the Sand Garden Home Assistant integration.
Creates a zen-inspired circular pattern icon representing sand garden art.
"""

from PIL import Image, ImageDraw
import os

def create_icon(size, filename):
    """Create a zen-inspired sand garden icon with concentric circular patterns."""
    # Create image with transparent background
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Background circle (sand color)
    sand_color = (245, 222, 179, 255)  # Wheat/sand color
    draw.ellipse([0, 0, size-1, size-1], fill=sand_color, outline=None)

    # Draw concentric spiral-like circles (representing sand patterns)
    center_x, center_y = size // 2, size // 2
    line_color = (139, 115, 85, 255)  # Darker brown for lines

    # Create multiple concentric circles with varying radii
    max_radius = size // 2 - size // 20
    num_circles = 8
    line_width = max(1, size // 128)

    for i in range(num_circles):
        radius = max_radius * (i + 1) / num_circles
        bbox = [
            center_x - radius,
            center_y - radius,
            center_x + radius,
            center_y + radius
        ]
        draw.ellipse(bbox, outline=line_color, width=line_width)

    # Add a small center dot (representing the starting point)
    dot_radius = size // 32
    dot_bbox = [
        center_x - dot_radius,
        center_y - dot_radius,
        center_x + dot_radius,
        center_y + dot_radius
    ]
    draw.ellipse(dot_bbox, fill=line_color, outline=None)

    # Save the image
    img.save(filename, 'PNG', optimize=True)
    print(f"Created {filename} ({size}x{size})")

def create_logo(width, height, filename):
    """Create a landscape logo with the sand garden pattern and text."""
    # Create image with transparent background
    img = Image.new('RGBA', (width, height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Background rectangle (sand color)
    sand_color = (245, 222, 179, 255)
    draw.rectangle([0, 0, width-1, height-1], fill=sand_color, outline=None)

    # Draw icon pattern on the left side
    icon_size = int(height * 0.8)
    margin = (height - icon_size) // 2

    center_x, center_y = margin + icon_size // 2, height // 2
    line_color = (139, 115, 85, 255)

    # Concentric circles
    max_radius = icon_size // 2 - 4
    num_circles = 6
    line_width = max(1, height // 64)

    for i in range(num_circles):
        radius = max_radius * (i + 1) / num_circles
        bbox = [
            center_x - radius,
            center_y - radius,
            center_x + radius,
            center_y + radius
        ]
        draw.ellipse(bbox, outline=line_color, width=line_width)

    # Center dot
    dot_radius = height // 32
    dot_bbox = [
        center_x - dot_radius,
        center_y - dot_radius,
        center_x + dot_radius,
        center_y + dot_radius
    ]
    draw.ellipse(dot_bbox, fill=line_color, outline=None)

    # Add decorative lines on the right (representing sand ripples)
    text_start_x = margin + icon_size + margin
    ripple_spacing = height // 8
    for i in range(3):
        y = height // 2 - ripple_spacing + i * ripple_spacing
        draw.line(
            [(text_start_x, y), (width - margin, y)],
            fill=line_color,
            width=line_width
        )

    # Save the image
    img.save(filename, 'PNG', optimize=True)
    print(f"Created {filename} ({width}x{height})")

def main():
    # Create output directory
    output_dir = 'custom_components/sand_garden'
    os.makedirs(output_dir, exist_ok=True)

    # Generate icons
    create_icon(256, os.path.join(output_dir, 'icon.png'))
    create_icon(512, os.path.join(output_dir, 'icon@2x.png'))

    # Generate logos (landscape format)
    create_logo(512, 256, os.path.join(output_dir, 'logo.png'))
    create_logo(1024, 512, os.path.join(output_dir, 'logo@2x.png'))

    print("\n✓ All icons and logos generated successfully!")
    print(f"  Files created in: {output_dir}/")

if __name__ == '__main__':
    main()
