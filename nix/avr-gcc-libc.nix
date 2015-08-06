{ stdenv, fetchurl, fetchsvn, texinfo, gmp, mpfr, libmpc, zlib, automake, autoconf, libtool }:

stdenv.mkDerivation {
  name = "avr-gcc-libc";

  srcs = [
    (fetchurl {
        url = "mirror://gnu/binutils/binutils-2.25.tar.bz2";
        sha256 = "08r9i26b05zcwb9zxb6zllpfdiiicdfsgbpsjlrjmvx3rxjzrpi2";
    })
    (fetchurl {
        url = "mirror://gcc/releases/gcc-5.2.0/gcc-5.2.0.tar.bz2";
        sha256 = "5f835b04b5f7dd4f4d2dc96190ec1621b8d89f2dc6f638f9f8bc1b1014ba8cad";
    })
    (fetchsvn {
        url = http://svn.savannah.nongnu.org/svn/avr-libc/trunk/avr-libc;
        rev = 2478;
        sha256 = "18rwc2c7yi85nndysp8pqwnici83x9cz791hfm572ya4yf9dv0i9";
    })
  ];
  
  sourceRoot = ".";

  nativeBuildInputs = [ texinfo automake autoconf libtool ];
  
  buildInputs = [ gmp mpfr libmpc zlib ];
  
  # Make sure we don't strip the libraries in lib/gcc/avr.
  stripDebugList= [ "bin" "avr/bin" "libexec" ];
  
  installPhase = ''
    # Make sure gcc finds the binutils.
    export PATH=$PATH:$out/bin
    
    pushd binutils*
    mkdir build && cd build
    ../configure --target=avr --prefix="$out" --disable-nls --disable-debug --disable-dependency-tracking
    make $MAKE_FLAGS
    make install
    popd

    pushd gcc*
    mkdir build && cd build
    ../configure --target=avr --prefix="$out" --disable-nls --disable-libssp --with-dwarf2 --disable-install-libiberty --with-system-zlib --enable-languages=c,c++
    make $MAKE_FLAGS
    make install
    popd

    # We don't want avr-libc to use the native compiler.
    export BUILD_CC=$CC
    export BUILD_CXX=$CXX
    unset CC
    unset CXX

    pushd avr-libc*
    ./bootstrap
    ./configure --prefix="$out" --build=`./config.guess` --host=avr
    make $MAKE_FLAGS
    make install
    popd
  '';
}