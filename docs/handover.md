# libreglass — LG TV Linux port: műszaki átadás

Állapot: 2026-11-26. A projekt neve és a GitHub-repó neve libreglass (korábbi munkanév: PanelBoot Linux).

## Cél és határok

A tulajdonos hosszú távú célja a webOS kiváltása saját, minimális, általa felügyelt Linuxszal, lehetőleg pendrive-on tárolt gyökérfájlrendszerrel. A cél, hogy ő dönthessen a futó szolgáltatásokról és a hálózati forgalomról. A gyári launcher lecserélése és a debloat önmagukban részmegoldások. A Glasshouse felülete lehetséges kiindulópont. A panel, HDMI, hang, tuner, távirányító, készenlét és védett streaming támogatása külön munkafázisok.

A TV a felhasználó tulajdona. A készüléken az engedélyezett vizsgálat csak olvasási műveletekre terjed ki. Indítási vagy flashművelethez külön egyeztetés kell, a tulajdonos jelenlétével és egyeztetett visszaállási lehetőséggel. A munkamenet során SSH-n keresztül, rendszergazdai jogosultsággal olvastunk, helyben töltöttünk le és fordítottunk, a készülékre nem került fájl, nem futott frissítés, újraindítás vagy kexec. A felhasználói kulcsot csak az SSH-kliens használta. Új kapcsolatnál az eredetileg megadott címet és identitást a tulajdonostól kérd, ha nem áll rendelkezésre. Az átadó fájlban nincsenek készülékazonosítók vagy hozzáférési adatok.

> Anonimizálás: a készülék- és hálózatazonosítók (cím, MAC, kulcs, felhasználónév) helyén helyőrzők állnak, például `<TV_IP>`, `<MAC_ADDR>`, `<SSH_KEY>`.

## Gyári adatok és SSH-leletek

- Modell: LG OLED55G19LA, 55 hüvelykes G1. Normál menü: 03.53.45. Instart szervizmenü: 03.53.45.01, Micom V5.06.1, boot 4.05.50/4.05.50, Chip Type O20N, Debug Status RELEASE, SIGN Key PRODKEY, UHD BE Version N/A, panelgyártó LGD.
- Device tree: LG Electronics, DTV SoC LG1212 (AArch64), compatible értékek `lge,lg1212` és `lge,lg1k`. Négy CPU, implementer `0x41` és part `0xd05`, vagyis Arm Cortex-A55.
- webOS TV 6.5.3, starfish. Rockhopper release 6.5.3-47 (kisscurl-koli). Gyári image-név: `lib32-starfish-global-secured`. `/etc/version`: 20250913080742. A futó ARM64 kernel `4.4.84-229.kcl4tv.6`, SMP PREEMPT, buildidő 2025-09-13 08:10:48 UTC, GCC 8.2.0.
- A gyári modellfájl márkát és gyártót adott. A dokumentált Luna rendszerinformációs lekérdezés ebben az SSH-munkamenetben nem adott használható választ. A modell és a szoftververzió a tulajdonos képeiről igazolt.
- Ugyanaz a Linux kernel látja a DVB frontendeket és demuxot (`lg1k-te`/`lg1k-dvb`), a HDMI-bemenetet, az eARC-ot és a videodekóder Video4Linux eszközeit. Fut az `utp-dvb`, `tvpowerd` és `micomservice`. `MCU_0_IPC_SEND` és `MCU_1_IPC_SEND` szálakat is láttunk. A külön verziózott Micom-firmware és a kommunikáció segédvezérlőkre utal. A panel és készenlét teljes vezérlési felosztása nyitott.
- Belső tároló: kb. 7,3 GiB eMMC, 56 számozott partíció, boot0/boot1 és RPMB. A bootpartíciók sysfs `force_ro` értéke 1. A kernel parancssora `root=/dev/mmcblk0p27 ro rootfstype=squashfs`, a mountinfo 179:27-et igazolt. A p28 a csak olvasható BSP SquashFS. A p29–p33 további gyári SquashFS összetevők. A p54/p55/p56 írható adatterületek, többek között adatbázis, felhasználói adatok és appok.
- A p27–p33 és p38–p44 azonos méretsora második rendszerpéldányra utal. A második példány tartalmát, slotváltást, visszaállást és recovery képességet nem teszteltük.
- A futó frissítő `/usr/sbin/update`. Az LG a modell támogatási oldalán listázza a `Software_File(Version_03.53.45).zip` csomagot. Az LG dokumentál USB-s frissítést modellhez kiadott EPK-val. Azonos verzió újratelepítése, downgrade, önálló USB-boot és egyedi EPK elfogadása nyitott kérdés. A `CONFIG_DM_VERITY` hiányából nem derül ki a bootloader ellenőrzési módszere.
- A vezetékes `eth0` driver `stmmaceth`. `CONFIG_KEXEC=y`, `CONFIG_KEXEC_CORE=y`, `kexec_load_disabled=0`. A PATH-ban a vizsgálatkor nem találtunk `kexec` parancsot. A konfiguráció és sysctl csak a lehetőséget igazolja. A kernelátadás tényleges működése, a szükséges jogosultságok és a hardver újrainicializálása ellenőrizetlen.
- `CONFIG_BLK_DEV_INITRD` nincs beállítva a futó kernelben. A célnak szánt új kernelhez terveztünk initramfs-támogatást. A gyári konfigurációban az `USB_STORAGE`, `EXT4_FS`, `STMMAC_ETH`, `STMMAC_PLATFORM`, `DEVTMPFS`, `IP_PNP` és `IP_PNP_DHCP` beépített. A PL011 konzol támogatása be van kapcsolva, a fizikai soros hozzáférés nem igazolt.

## Forráscsomag és fordítás

- Az LG katalógusbejegyzése webOS 6.0 ON 1.0, azonosító 510316. Mindhárom archívum letöltődött és a tartalomjegyzéket ellenőriztük. Az 1-es archívum a `common/` könyvtár felhasználói forráscsomagjait tartalmazza, a 3-as a `chromium79/` anyagát. A 2-es archívum tartalmazza a `soc/o20n_mt7921.tgz` fájlt.
- A belső archívumban a `linux-rockhopper_build_o20` alatt találtunk generált konfigurációt, `Module.symvers` fájlt és az `arch/arm64/boot/dts/lg1212-aarch64.dts` fájlt. A generált `utsrelease.h` szerint a közzétett build 4.4.84-214. A `kernel-source` alatt nulla `.c` fájlt listáztunk, a `vmlinux` hivatkozás célját sem találtuk ebben az archívumban.
- Az Arm GNU 8.2-2019.01 AArch64 fordító helyben működött, GCC 8.2.1. A TV konfigurációjából előkészített helyi buildben a `BLK_DEV_INITRD` és `RD_GZIP` beállításokat kapcsoltuk be, a tervezett verzió-utótag `-g1-poc`. A `make olddefconfig` a `scripts/basic/fixdep.c` hiányán állt meg. Fordított O20N `Image` nem készült.
- A tulajdonos elküldte az LG Open Source megkeresést a 03.53.45.01 firmware-ben terjesztett `4.4.84-229.kcl4tv.6` kernel teljes megfelelő forrásáért. Kérte a gyártói módosításokat, kernelkonfigurációt, eszközfát és build/telepítési szkripteket. A következő input az LG válasza vagy az általa megadott letöltés. Nyilvánosan elérhető, igazolt, teljes és egyező forrást eddig nem találtunk. A GPLv2 3. pontja a terjesztett kernelbinárisnak megfelelő forrásról szól.

## Helyi PoC és ellenőrzés

A vizsgálati fájlok eredeti helye `<POC_WORKDIR>` (például `/tmp/lg-g1-poc.XXXXXX`). Ez ideiglenes helyi könyvtár. Másik gépen külön át kell vinni a szükséges fájlokat. A futó eszközfa a konkrét TV-ről jött. Megosztás előtt ellenőrizni kell benne a készülékhez kötött tulajdonságokat és a `/chosen` adatokat.

- `running-config.gz` — a `/proc/config.gz` ellenőrzött példánya. SHA-256 `3f8464ce1c1b5543a233fd21fc0f044944bcddf7003becdb6c36ff1e13105fe2`
- `running.dtb` — a `/sys/firmware/fdt` példánya, érvényes DTB v17, 532480 bájt. SHA-256 `<SHA256 of device-specific DTB, not published>`. A DTB készülékenkénti tulajdonságokat és `/chosen` adatokat tartalmaz, ezért nem publikus.
- `poc-init.c` — az init forrása: [poc/poc-init.c](poc/poc-init.c), az átadás végén (melléklet) is szerepel.
- `poc-initramfs.cpio.gz` — gzip/newc archívum, `dev/` és statikusan linkelt ARM64 `init`. SHA-256 `8efdabe7775abaace9ddbf8a88074885129c121c1ca64d1b08d48a655382447c`
- `url-1.tar.gz`, `url-2.tar.gz`, `url-3.tar.gz` — az LG archívumai. `o20n-files.txt` és `common-files.txt` — helyi tartalomjegyzékek. Az `url-*.json` ideiglenes aláírt letöltési hivatkozásokat tárolt. Új letöltéshez az LG katalógust használd.

A helyi x86 futás ellenőrizte az init jelzőszövegét a nem PID 1 ágban. Az ARM64 ELF statikus linkelését és a CPIO tartalmát ellenőriztük. A PID 1 ág, a konzol, a DHCP és a UDP broadcast a TV-n még nem futott. A TV-n indított init a konzolra próbál írni, majd 3 másodpercenként UDP broadcast jelzést próbál küldeni az 45555-ös portra. Az init maga nem állít IP-címet. A hálózati jelhez külön működő interfész és címbeállítás kell.

## Következő mérföldkövek

1. LG forráscsomag ellenőrzése. A verzióazonosító, kernel C-fájlok, platformdriverek, eszközfa és buildutasítások legyenek jelen. A gyári kiinduló konfigurációt hasonlítsd a TV-ről mentetthez.
2. Helyi ARM64 Image és initramfs előállítása, a platformhoz illő eszközfa és a beépített ethernet driver ellenőrzése. Az elkészült Image önmagában még nem bizonyítja az indíthatóságot.
3. Élő próba előtt külön egyeztetés a tulajdonossal, aki eléri a készülék tápját. Első sikerfeltétel a saját kernelből indult init egyértelmű konzolos vagy LAN-jelzése, majd a gyári webOS visszatérése normál hidegindítással. A PoC tárolóírás nélküli.
4. Sikeres RAM-próba után USB gyökérfájlrendszer. A későbbi automatizálás külön feladat. Közvetlen USB-boot esetén a webOS nem indulna el. Gyári boot utáni kexec esetén a webOS az átadásig futna. A gyári kernel alatti chroot esetén a webOS tovább futna, így ez nem éri el a projekt végső célját.

## Hordozhatóság és más modellek

- A közös ARM64 gyökérfájlrendszer, UI és alkalmazások több eszköz között megoszthatók. A kernel/eszközfa, boot és hardverfunkciók támogatását platform és készülékváltozat szerint kell igazolni. A stock webOS alatti hagyományos debloathoz általában nem kell kernelforrás, viszont a szolgáltatás- és alkalmazásazonosítók firmware-függők.
- G1 OLED65G13LA: publikált mérés szerint azonos O20N/LG1212 és `4.4.84-229.kcl4tv.6`. A beszámoló webOS alatt futó alkalmazást igazol. A panel és az eszközfa külön ellenőrzendő. <https://github.com/Hubert-Rybak/chiaki-lg-fork#tested-on>
- C1 OLED65C17LB: közeli O20-változat. Publikált próbában a kernelátadás sikertelen volt. <https://github.com/RodriguezCF/LG-C1-kernel>
- C2 OLED65C21LA: publikált PoC módosított gyári kernellel, kexec utáni USB-ről futó Alpine-nal, Ethernettel és alap grafikával/hanggal. A projekt korlátozott működést jelentett, többek között újraindítási és grafikai problémákkal. <https://github.com/Pokropow/LG-TV-OSS>
- C4: az LG α9 Gen7 és webOS 24 termékként dokumentálta. Az OLED55C4PUA egy konkrét firmware-ét a Glasshouse listázza. Ez webOS alatti támogatás. C4 saját kernelindítására nem találtunk közvetlen igazolást. A C2 bootképe nincs C4-re ellenőrizve. <https://www.lg.com/at/tv-soundbars/oled-evo/oled55c47la/> | <https://rorygallagher2024.github.io/glasshouse/tested-tvs/>

## Glasshouse és Widevine

A Glasshouse Home webOS 6-on saját alkalmazásként fut, a Home gomb átirányítható rá. Ilyenkor a DRM-es gyári alkalmazások a webOS lejátszási láncát használják. Saját Linuxhoz a felület portolható vizsgálati irány. Az eredeti backend többek között webOS Luna-szolgáltatásokra, appkezelésre, bemenetvezérlésre és a gyári grafikus rétegre támaszkodik. Ezekhez saját Linuxon más implementáció kell. <https://rorygallagher2024.github.io/glasshouse/dashboard/home/> | <https://rorygallagher2024.github.io/glasshouse/IMPLEMENTATION/>

Az Instart menü jelzi a gyári Widevine provisionálást. A TV-tanúsítvány önmagában nem ad saját OS-en gyári minőségű lejátszást. A készülékintegrációhoz kompatibilis DRM-kliens, OEMCrypto, szükség szerint TEE és védett videóút, valamint a szolgáltató támogatása is tartozik. A saját Linuxon való használat nem igazolt. A streaming külön mérföldkő. <https://developers.google.com/widevine/drm/overview> | <https://webostv.developer.lge.com/develop/guides/drm-content-playback>

## Elsődleges hivatkozások

- LG modelltámogatás: <https://www.lg.com/at/service/produktsupport/cs-OLED55G19LA.AEU>
- LG modell szerinti forráskatalógus: <https://opensource.lge.com/product/list?ctgr=005&subCtgr=006&keyword=OLED55G19LA&schType=1>
- LG modell szerinti forráskérés: <https://opensource.lge.com/inquiry?osSeq=510316&modelName=OLED55G19LA&osCategoryId=005&osSubcategoryId=006>
- GPLv2 3. pont: <https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html>
- Arm Cortex-A55 kézikönyv: <https://documentation-service.arm.com/static/5e7e09f6a3736a0d2e862d2f>
- Kernel kexec sysctl: <https://www.kernel.org/doc/html/v6.5/admin-guide/sysctl/kernel.html#kexec-load-disabled>
- LG USB-frissítés: <https://www.lg.com/us/support/help-library/lg-tv-how-to-manually-update-software-CT10000018-20154165668198>

## Melléklet — a PoC init forrása

Fájlként: [poc/poc-init.c](poc/poc-init.c).

```c
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mount.h>
#include <sys/socket.h>
#include <unistd.h>

static const char marker[] = "LG-G1-POC: init reached\n";

int main(void)
{
    if (getpid() != 1)
        return fputs(marker, stdout) == EOF;

    if (mount("devtmpfs", "/dev", "devtmpfs", MS_NOSUID | MS_NOEXEC, NULL) == 0) {
        int console = open("/dev/console", O_WRONLY | O_NOCTTY);
        if (console >= 0) {
            write(console, marker, sizeof(marker) - 1);
            close(console);
        }
    }

    int fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    int broadcast = 1;
    struct sockaddr_in destination = {
        .sin_family = AF_INET,
        .sin_port = htons(45555),
        .sin_addr.s_addr = htonl(INADDR_BROADCAST),
    };
    if (fd >= 0)
        setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

    for (;;) {
        if (fd >= 0)
            sendto(fd, marker, sizeof(marker) - 1, 0,
                   (const struct sockaddr *)&destination, sizeof(destination));
        sleep(3);
    }
}
```
