set -e

if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root"
   exit 1
fi

apt install -y          \
    g++                 \
    cmake               \
    curl                \
    tar

# boost 1.86.0
mkdir -p third_party
cd third_party

curl -LO https://archives.boost.io/release/1.86.0/source/boost_1_86_0.tar.gz

if [[ -d boost ]]; then
    echo "Removing existing boost directory..."
    rm -rf boost
fi
mkdir boost

tar -xzf boost_1_86_0.tar.gz -C boost --strip-components=1
rm boost_1_86_0.tar.gz

cd boost
./bootstrap.sh --prefix=/usr
./b2 install

./b2 install