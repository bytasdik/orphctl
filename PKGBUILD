# Maintainer: bytasdik <your@email.com>
pkgname=orphctl
pkgver=1.0.0
pkgrel=1
pkgdesc="Automatic orphaned package remover for Arch Linux with systemd timer support"
arch=('x86_64')
url="https://github.com/bytasdik/orphctl"
license=('MIT')
depends=('pacman' 'systemd')
optdepends=('yay: for AUR cache cleanup via --aur flag')
source=("$pkgname-$pkgver-x86_64.tar.gz::$url/releases/download/v$pkgver/$pkgname-$pkgver-x86_64.tar.gz")
sha256sums=('REPLACEME')  # paste output of: sha256sum orphctl-1.0.0-x86_64.tar.gz

package() {
    install -Dm755 orphctl "$pkgdir/usr/bin/orphctl"
    install -Dm644 "$srcdir/../LICENSE" "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
    install -Dm644 "$srcdir/../README.md" "$pkgdir/usr/share/doc/$pkgname/README.md"
}
