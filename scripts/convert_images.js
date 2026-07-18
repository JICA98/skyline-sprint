const { Jimp } = require('jimp');
const path = require('path');
const fs = require('fs');

const assets = [
    {
        src: '/home/jica/.gemini/antigravity-cli/brain/2cb5ca1d-b18a-4e78-aa9f-eb3f7f8b0cbb/runner_idle_1784351904726.jpg',
        dest: '/home/jica/repo/skyline-sprint/package/assets/images/runner_idle.tga'
    },
    {
        src: '/home/jica/.gemini/antigravity-cli/brain/2cb5ca1d-b18a-4e78-aa9f-eb3f7f8b0cbb/runner_run_1784351917861.jpg',
        dest: '/home/jica/repo/skyline-sprint/package/assets/images/runner_run.tga'
    },
    {
        src: '/home/jica/.gemini/antigravity-cli/brain/2cb5ca1d-b18a-4e78-aa9f-eb3f7f8b0cbb/runner_jump_1784351932064.jpg',
        dest: '/home/jica/repo/skyline-sprint/package/assets/images/runner_jump.tga'
    },
    {
        src: '/home/jica/.gemini/antigravity-cli/brain/2cb5ca1d-b18a-4e78-aa9f-eb3f7f8b0cbb/spacesuit_idle_1784351950363.jpg',
        dest: '/home/jica/repo/skyline-sprint/package/assets/images/spacesuit_idle.tga'
    },
    {
        src: '/home/jica/.gemini/antigravity-cli/brain/2cb5ca1d-b18a-4e78-aa9f-eb3f7f8b0cbb/spacesuit_run_1784351963500.jpg',
        dest: '/home/jica/repo/skyline-sprint/package/assets/images/spacesuit_run.tga'
    },
    {
        src: '/home/jica/.gemini/antigravity-cli/brain/2cb5ca1d-b18a-4e78-aa9f-eb3f7f8b0cbb/spacesuit_jump_1784351977997.jpg',
        dest: '/home/jica/repo/skyline-sprint/package/assets/images/spacesuit_jump.tga'
    },
    {
        src: '/home/jica/.gemini/antigravity-cli/brain/2cb5ca1d-b18a-4e78-aa9f-eb3f7f8b0cbb/platform_tile_1784350944774.jpg',
        dest: '/home/jica/repo/skyline-sprint/package/assets/images/platform.tga'
    },
    {
        src: '/home/jica/.gemini/antigravity-cli/brain/2cb5ca1d-b18a-4e78-aa9f-eb3f7f8b0cbb/enemy_sprite_1784350960281.jpg',
        dest: '/home/jica/repo/skyline-sprint/package/assets/images/enemy.tga'
    }
];

async function convert() {
    for (const asset of assets) {
        console.log(`Loading: ${asset.src}`);
        const image = await Jimp.read(asset.src);
        
        const width = image.bitmap.width;
        const height = image.bitmap.height;
        const rawBuffer = image.bitmap.data; // RGBA buffer

        console.log(`Converting to TGA (width=${width}, height=${height})...`);

        // Convert RGBA to BGRA with transparency check (except platform)
        const isTransparentAsset = !asset.dest.includes('platform.tga');
        const bgraBuffer = Buffer.alloc(width * height * 4);
        for (let i = 0; i < rawBuffer.length; i += 4) {
            const r = rawBuffer[i];
            const g = rawBuffer[i + 1];
            const b = rawBuffer[i + 2];
            let a = rawBuffer[i + 3];

            if (isTransparentAsset && r < 40 && g < 40 && b < 40) {
                a = 0; // Transparent background
            }

            bgraBuffer[i] = b;     // B
            bgraBuffer[i + 1] = g; // G
            bgraBuffer[i + 2] = r; // R
            bgraBuffer[i + 3] = a; // A
        }

        // TGA Header (18 bytes)
        const header = Buffer.alloc(18);
        header[2] = 2; // Uncompressed true-color
        header[12] = width & 0xFF;
        header[13] = (width >> 8) & 0xFF;
        header[14] = height & 0xFF;
        header[15] = (height >> 8) & 0xFF;
        header[16] = 32; // 32 bits per pixel (BGRA)
        header[17] = 8 | 32; // 8 bits alpha, top-to-bottom

        console.log(`Writing to: ${asset.dest}`);
        fs.mkdirSync(path.dirname(asset.dest), { recursive: true });
        
        // Write TGA file
        fs.writeFileSync(asset.dest, Buffer.concat([header, bgraBuffer]));
    }
    console.log("All images successfully converted to uncompressed TGA format!");
}

convert().catch(err => {
    console.error("Conversion failed:", err);
    process.exit(1);
});
