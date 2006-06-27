; #######################################
; slovak.nsh
; slovak language strings for inkscape installer
; windows code page: 1250
; Authors:
; helix84 helix84@gmail.com (translation for Inkscape 0.44)
;
!insertmacro MUI_LANGUAGE "Slovak"

; Product name
LangString lng_Caption   ${LANG_SLOVAK} "${PRODUCT_NAME} -- Open Source Scalable Vector Graphics Editor"

; Button text "Next >" for the license page
LangString lng_LICENSE_BUTTON   ${LANG_SLOVAK} "�alej >"

; Bottom text for the license page
LangString lng_LICENSE_BOTTOM_TEXT   ${LANG_SLOVAK} "$(^Name) je mo�n� ��ri� za podmienok General Public License (GPL). Licen�n� zmluva je tu len pre informa�n� ��ely. $_CLICK"

;has been installed by different user
LangString lng_DIFFERENT_USER ${LANG_SLOVAK} "Inkscape nain�taloval pou��vate� $0.$\r$\nIn�tal�cia nemus� spr�vne skon�i�, ak v nej budete pokra�ova�!$\r$\nPros�m, prihl�ste sa ako $0 a spustite in�tal�ciu znova."

;you have no admin rigths
LangString lng_NO_ADMIN ${LANG_SLOVAK} "Nem�te administr�torsk� pr�va.$\r$\nIn�tal�cia Inkscape pre v�etk�ch pou��vate�ov nemus� skon�i� �spe�ne.$\r$\nZru�te ozna�enie vo�by 'Pre v�etk�ch pou��vate�ov'."

;win9x is not supported
LangString lng_NOT_SUPPORTED ${LANG_SLOVAK} "Inkscape neb�� na Windows 95/98/ME!$\r$\nPodrobnej�ie inform�cie n�jdete na ofici�lnom webe."

; Full install type
LangString lng_Full $(LANG_SLOVAK) "Pln�"

; Optimal install type
LangString lng_Optimal $(LANG_SLOVAK) "Optim�lna"

; Minimal install type
LangString lng_Minimal $(LANG_SLOVAK) "Minim�lna"

; Core install section
LangString lng_Core $(LANG_SLOVAK) "${PRODUCT_NAME} SVG editor (povinn�)"

; Core install section description
LangString lng_CoreDesc $(LANG_SLOVAK) "S�bory a kni�nice ${PRODUCT_NAME}"

; GTK+ install section
LangString lng_GTKFiles $(LANG_SLOVAK) "GTK+ runtime environment (povinn�)"

; GTK+ install section description
LangString lng_GTKFilesDesc $(LANG_SLOVAK) "Multiplatformov� sada pou��vate�sk�ho rozhrania pou�it�ho v ${PRODUCT_NAME}"

; shortcuts install section
LangString lng_Shortcuts $(LANG_SLOVAK) "Z�stupcovia"

; shortcuts install section description
LangString lng_ShortcutsDesc $(LANG_SLOVAK) "Z�stupcovia pre �tart ${PRODUCT_NAME}"

; All user install section
LangString lng_Alluser $(LANG_SLOVAK) "pre v�etk�ch pou��vate�ov"

; All user install section description
LangString lng_AlluserDesc $(LANG_SLOVAK) "In�talova� aplik�ciu pre kohoko�vek, kto pou��va tento po��ta�. (v�etci pou��vatelia)"

; Desktop section
LangString lng_Desktop $(LANG_SLOVAK) "Plocha"

; Desktop section description
LangString lng_DesktopDesc $(LANG_SLOVAK) "Vytvo�it z�stupcu ${PRODUCT_NAME} na ploche"

; Start Menu  section
LangString lng_Startmenu $(LANG_SLOVAK) "Ponuka �tart"

; Start Menu section description
LangString lng_StartmenuDesc $(LANG_SLOVAK) "Vytvori� pre ${PRODUCT_NAME} polo�ku ve ponuke �tart"

; Quick launch section
LangString lng_Quicklaunch $(LANG_SLOVAK) "Panel r�chl�ho spustenia"

; Quick launch section description
LangString lng_QuicklaunchDesc $(LANG_SLOVAK) "Vytvori� pre ${PRODUCT_NAME} z�stupcu v paneli r�chleho spustenia"

; File type association for editing
LangString lng_SVGWriter ${LANG_SLOVAK} "Otv�ra� SVG s�bory v ${PRODUCT_NAME}"

; File type association for editing description
LangString lng_SVGWriterDesc ${LANG_SLOVAK} "Vybra� ${PRODUCT_NAME} ako v�chodz� editor pre SVG soubory"

; Context Menu
LangString lng_ContextMenu ${LANG_SLOVAK} "Kontextov� menu"

; Context Menu description
LangString lng_ContextMenuDesc ${LANG_SLOVAK} "Prida� ${PRODUCT_NAME} do kontextov�ho menu pre SVG s�bory"


; Additional files section
LangString lng_Addfiles $(LANG_SLOVAK) "�al�ie s�bory"

; Additional files section description
LangString lng_AddfilesDesc $(LANG_SLOVAK) "�al�ie s�bory"

; Examples section
LangString lng_Examples $(LANG_SLOVAK) "Pr�klady"

; Examples section description
LangString lng_ExamplesDesc $(LANG_SLOVAK) "Pr�lady pou��v�nia ${PRODUCT_NAME}"

; Tutorials section
LangString lng_Tutorials $(LANG_SLOVAK) "Sprievodcovia"

; Tutorials section description
LangString lng_TutorialsDesc $(LANG_SLOVAK) "Sprievodcovia funkciami ${PRODUCT_NAME}"


; Languages section
LangString lng_Languages $(LANG_SLOVAK) "Jazykov� sady"

; Languages section dscription
LangString lng_LanguagesDesc $(LANG_SLOVAK) "Nain�talova� �al�ie jazykov� sady ${PRODUCT_NAME}"

LangString lng_am $(LANG_SLOVAK) "am  Amharic"
LangString lng_az $(LANG_SLOVAK) "az  Azerbaijani"
LangString lng_be $(LANG_SLOVAK) "be  Byelorussian"
LangString lng_ca $(LANG_SLOVAK) "ca  Catalan"
LangString lng_cs $(LANG_SLOVAK) "cs  Czech"
LangString lng_da $(LANG_SLOVAK) "da  Danish"
LangString lng_de $(LANG_SLOVAK) "de  German"
LangString lng_el $(LANG_SLOVAK) "el  Greek"
LangString lng_en $(LANG_SLOVAK) "en  English"
LangString lng_es $(LANG_SLOVAK) "es  Spanish"
LangString lng_es_MX $(LANG_SLOVAK) "es_MX  Mexican Spanish"
LangString lng_et $(LANG_SLOVAK) "es  Estonian"
LangString lng_fr $(LANG_SLOVAK) "fr  French"
LangString lng_ga $(LANG_SLOVAK) "ga  Irish"
LangString lng_gl $(LANG_SLOVAK) "gl  Gallegan"
LangString lng_hu $(LANG_SLOVAK) "hu  Hungarian"
LangString lng_it $(LANG_SLOVAK) "it  Italian"
LangString lng_ja $(LANG_SLOVAK) "ja  Japanese"
LangString lng_ko $(LANG_SLOVAK) "ko  Korean"
LangString lng_lt $(LANG_SLOVAK) "lt  Lithuanian"
LangString lng_mk $(LANG_SLOVAK) "mk  Macedonian"
LangString lng_nb $(LANG_SLOVAK) "nb  Norwegian Bokmal"
LangString lng_nl $(LANG_SLOVAK) "nl  Dutch"
LangString lng_nn $(LANG_SLOVAK) "nn  Norwegian Nynorsk"
LangString lng_pa $(LANG_SLOVAK) "pa  Panjabi"
LangString lng_pl $(LANG_SLOVAK) "po  Polish"
LangString lng_pt $(LANG_SLOVAK) "pt  Portuguese"
LangString lng_pt_BR $(LANG_SLOVAK) "pt_BR Brazilian Portuguese"
LangString lng_ru $(LANG_SLOVAK) "ru  Russian"
LangString lng_sk $(LANG_SLOVAK) "sk  Slovak"
LangString lng_sl $(LANG_SLOVAK) "sl  Slovenian"
LangString lng_sr $(LANG_SLOVAK) "sr  Serbian"
LangString lng_sr@Latn $(LANG_SLOVAK) "sr@Latn  Serbian in Latin script"
LangString lng_sv $(LANG_SLOVAK) "sv  Swedish"
LangString lng_tr $(LANG_SLOVAK) "tr  Turkish"
LangString lng_uk $(LANG_SLOVAK) "uk  Ukrainian"
LangString lng_vi $(LANG_SLOVAK) "vi  Vietnamese"
LangString lng_zh_CN $(LANG_SLOVAK) "zh_CH  Simplifed Chinese"
LangString lng_zh_TW $(LANG_SLOVAK) "zh_TW  Traditional Chinese"




; uninstallation options
LangString lng_UInstOpt   ${LANG_SLOVAK} "Mo�nosti dein�tal�cie"

; uninstallation options subtitle
LangString lng_UInstOpt1  ${LANG_SLOVAK} "Zvo�te pros�m �al�ie mo�nosti"

; Ask to purge the personal preferences
LangString lng_PurgePrefs ${LANG_SLOVAK} "Ponecha� osobn� nastavenia"
