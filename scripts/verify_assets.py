import hashlib
import sys
import os

EXPECTED_CHECKSUMS = {
    "package/assets/audio/failure.wav": "5702f620b85229f46e9717a71435d5b45337368853f1f87d7238a389fb5a3406",
    "package/assets/audio/jump.wav": "e279082f9ecc71cbd36532183d3958c73979955a7c6c193f60bf0b69d3f913a0",
    "package/assets/audio/pickup.wav": "bc12ca1f4a1a5eac10a68765b40ff96ad3cd2a1503e6376fdae06d9a915b221c",
    "package/assets/images/enemy.tga": "fd4614a725874133ccae2a8dfe61389a1914a1da8f8591089cdc45d49060baca",
    "package/assets/images/font.tga": "857260853c33470f1d2f305952233d8d1181be04613c1423da62216cea5f0cfd",
    "package/assets/images/platform.tga": "0951ae08eff2957eca71c77c1e54820b4516d7b2b95fc9d62adb00afd30fcecf",
    "package/assets/images/player.tga": "ad94b4e9d5cd2fe9d953cf698e333fde03e2ddd5318ea2b39ea2542e6da43e04",
    "package/assets/images/shard.tga": "2eb218d28d10441cba7073e1157c2a5007742861db8faa363555f7cecc759fb8",
    "package/assets/images/splash.tga": "d8098e033568cf6f1708ddc6264c5d8efaa0cc6697a004b61cde8f92c2281f79",
    "package/assets/images/warning.tga": "6c19ff9f89192bb853e055afe70fcbea453714573cf16c6f672da6c5a29f2972"
}

def get_sha256(path):
    h = hashlib.sha256()
    try:
        with open(path, "rb") as f:
            while chunk := f.read(8192):
                h.update(chunk)
        return h.hexdigest()
    except FileNotFoundError:
        return None

def verify():
    all_ok = True
    print("Verifying Skyline Sprint assets...")
    for path, expected in EXPECTED_CHECKSUMS.items():
        actual = get_sha256(path)
        if actual is None:
            print(f"[-] {path}: MISSING")
            all_ok = False
        elif actual != expected:
            print(f"[-] {path}: CHECKSUM MISMATCH")
            print(f"    Expected: {expected}")
            print(f"    Actual:   {actual}")
            all_ok = False
        else:
            print(f"[+] {path}: OK")
            
    if all_ok:
        print("[+] All assets verified successfully.")
        sys.exit(0)
    else:
        print("[-] Asset verification failed!")
        sys.exit(1)

if __name__ == "__main__":
    verify()
