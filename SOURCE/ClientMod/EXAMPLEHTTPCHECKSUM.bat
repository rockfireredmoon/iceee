REM   An example script to generate an up to date HTTPChecksum.txt from
REM   existing CAR files, and copying them to potential locations.

md5 EarthEternal.car \Release\Current > HTTPChecksum.txt
md5 Catalogs.car \Release\Current\Media >> HTTPChecksum.txt
md5 Prop-ModAddons1.car \Release\Current\Media >> HTTPChecksum.txt
md5 Sound-ModSound.car \Release\Current\Media >> HTTPChecksum.txt
md5 Prop-ModWantedShadow.car \Release\Current\Media >> HTTPChecksum.txt

copy /y HTTPChecksum.txt C:\Server\Data

copy /y EarthEternal.car F:\EEAsset\86\Release\Current\EarthEternal.car
copy /y Catalogs.car F:\EEAsset\86\Release\Current\Media\Catalogs.car
copy /y Prop-ModAddons1.car F:\EEAsset\86\Release\Current\Media\Prop-ModAddons1.car
copy /y Sound-ModSound.car F:\EEAsset\86\Release\Current\Media\Sound-ModSound.car
copy /y Prop-ModWantedShadow.car F:\EEAsset\86\Release\Current\Media