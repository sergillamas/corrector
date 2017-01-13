# Corrector
This repository contains source files that can be integrated into FreeLing in order to create a word embeddings based automatic corrector.

In order to use these files, first you will need to have [Freeling](https://github.com/TALP-UPC/FreeLing) installed. You will also need word embeddings trained models, which you can search on internet. If you want models trained for Spanish, you can find some in this [repository](https://github.com/sergillamas/esp-w2v-models).

To install the files, you will need to manually move the files from the FreeLing folder (this project) to your copy of the FreeLing source files, and then recompile FreeLing. You need to move/substitute the files individually, not just replace the whole FreeLing folder.

The precision of this corrector is quite low (about 40%), and it's designed to study the application of word embeddings to automatic correction taking into account the whole context of the sentence to be corrected.

Other folders contain additional files used for the study, but that are not required to use the corrector.