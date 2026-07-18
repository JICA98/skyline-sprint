const { Jimp } = require('jimp');

async function main() {
    console.log("Reading BMP screenshot...");
    const image = await Jimp.read('/home/jica/.gemini/antigravity-cli/brain/2cb5ca1d-b18a-4e78-aa9f-eb3f7f8b0cbb/.user_uploaded/uploaded_media_1784351660346.bmp');
    console.log("Writing PNG screenshot...");
    await image.write('/home/jica/.gemini/antigravity-cli/brain/2cb5ca1d-b18a-4e78-aa9f-eb3f7f8b0cbb/screenshot2.png');
    console.log("Successfully converted screenshot to PNG format!");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
