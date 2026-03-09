# Maintainer: Your Name <your@email.com>
pkgname=orphctl
pkgver=1.0.0
pkgrel=1
pkgdesc="Automatic orphaned package remover for Arch Linux with systemd timer support"
arch=('x86_64')
url="https://github.com/bytasdik/orphctl"
license=('Unlicense')
depends=('pacman' 'systemd')
optdepends=('yay: for AUR cache cleanup support')
source=("$pkgname-$pkgver.tar.gz::$url/archive/refs/tags/v$pkgver.tar.gz")
sha256sums=('5b3794721040c5bc428f6f6113ae6018c9374e0aa3e17b7cd1f3edff82f16040')  # Replace with real checksum after uploading to GitHub

build() {
    cd "$pkgname-$pkgver"
    g++ -O2 -o orphctl orphctl.cpp
}

package() {
    cd "$pkgname-$pkgver"

    # Install the binary
    install -Dm755 orphctl "$pkgdir/usr/bin/orphctl"

    # Install license
    install -Dm644 LICENSE "$pkgdir/usr/share/licenses/$pkgname/LICENSE"

    # Install README
    install -Dm644 README.md "$pkgdir/usr/share/doc/$pkgname/README.md"
}
