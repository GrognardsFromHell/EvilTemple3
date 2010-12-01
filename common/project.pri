
CONFIG += debug_and_release warn_on build_all

OUTDIR = $$PWD/../out

CONFIG(debug, debug|release) {
	DESTDIR = $$OUTDIR/debug/bin/
} else {
	DESTDIR = $$OUTDIR/release/bin/
}
