CONFIG += ordered 
TEMPLATE = subdirs
SUBDIRS = \
    lib \
    view \
    plugin \
    viewer \
    data \

# Dont build benchmark for Qt 5, as it implicitly depends on QWidget still.
# Once libmaliit-keyboard has been cleaned up (QPixmap usage), benchmark can
# use a QCoreApplication and be compiled for Qt 5 again.
contains(QT_MAJOR_VERSION, 4) {
    SUBDIRS += benchmark
} 
