#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define HASH_SIZE 101
#define MAX_STR 64
#define MAX_ADDR 256
#define MAX_LINE 512

/* Ilac yapisi */
typedef struct Ilac {
    char isim[MAX_STR];
    int fiyat;
    int stok;
    struct Ilac* next;
} Ilac;

/* Siparis yapisi */
typedef struct Siparis {
    char ilacAdi[MAX_STR];
    int adet;
    int araToplam;
    struct Siparis* next;
} Siparis;

/* Sepet yapisi */
typedef struct {
    Siparis *bas, *son;
} Sepet;

/* Gecmis yapisi */
typedef struct Islem {
    char logMesaji[256];
    struct Islem* next;
} Islem;

/* Kullanici yapisi */
typedef struct Kullanici {
    char adSoyad[100];
    char adres[MAX_ADDR];
} Kullanici;

/* Global degiskenler */
Ilac* hashTable[HASH_SIZE] = {0};
Sepet sepetim = {NULL, NULL};
Islem* gecmisTop = NULL;
Kullanici aktifKullanici = {"", ""};
int genelToplam = 0;

/* Arayuz nesneleri */
HWND hTitle;
HWND hNameEdit, hAddressEdit;
HWND hSearchEdit, hQtyEdit;
HWND hResultLabel, hCartList, hHistoryList, hTotalLabel, hStockList;
HWND hBtnSearch, hBtnAdd, hBtnPay, hBtnRefresh, hBtnSaveUser;

/* Fontlar */
HFONT hFontNormal, hFontBold, hFontTitle;

/* Renk fircalari */
HBRUSH hBrushBg;
HBRUSH hBrushTitle;
HBRUSH hBrushButtonBlue;
HBRUSH hBrushButtonGreen;
HBRUSH hBrushButtonOrange;
HBRUSH hBrushButtonPurple;

/* Renkler */
COLORREF clrBg = RGB(242, 246, 252);
COLORREF clrTitle = RGB(44, 62, 80);
COLORREF clrTextDark = RGB(33, 37, 41);
COLORREF clrBlue = RGB(52, 152, 219);
COLORREF clrGreen = RGB(46, 204, 113);
COLORREF clrOrange = RGB(243, 156, 18);
COLORREF clrPurple = RGB(155, 89, 182);
COLORREF clrWhite = RGB(255, 255, 255);

/* Buton id */
#define ID_BTN_SEARCH   101
#define ID_BTN_ADD      102
#define ID_BTN_PAY      103
#define ID_BTN_REFRESH  104
#define ID_BTN_SAVEUSER 105

/* Metni kucult */
void metniKucult(char* s) {
    int i;
    for (i = 0; s[i]; i++) {
        s[i] = (char)tolower((unsigned char)s[i]);
    }
}

/* Hash uret */
unsigned int hashUret(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % HASH_SIZE;
}

/* Gecmise ekle */
void gecmiseEkle(const char* mesaj) {
    Islem* yeni = (Islem*)malloc(sizeof(Islem));
    if (!yeni) return;
    strcpy(yeni->logMesaji, mesaj);
    yeni->next = gecmisTop;
    gecmisTop = yeni;
}

/* Gecmisi temizle */
void gecmisiTemizle() {
    while (gecmisTop) {
        Islem* sil = gecmisTop;
        gecmisTop = gecmisTop->next;
        free(sil);
    }
}

/* Ilac ekle */
void sistemeIlacEkle(char* isim, int fiyat, int stok) {
    char temp[MAX_STR];
    unsigned int index;
    Ilac* yeni;

    strcpy(temp, isim);
    metniKucult(temp);
    index = hashUret(temp);

    yeni = (Ilac*)malloc(sizeof(Ilac));
    if (!yeni) return;

    strcpy(yeni->isim, isim);
    yeni->fiyat = fiyat;
    yeni->stok = stok;
    yeni->next = hashTable[index];
    hashTable[index] = yeni;
}

/* Ilac bul */
Ilac* ilacBul(char* isim) {
    char aranan[MAX_STR];
    unsigned int index;
    Ilac* gezgin;

    strcpy(aranan, isim);
    metniKucult(aranan);

    index = hashUret(aranan);
    gezgin = hashTable[index];

    while (gezgin) {
        char mevcut[MAX_STR];
        strcpy(mevcut, gezgin->isim);
        metniKucult(mevcut);
        if (strcmp(mevcut, aranan) == 0) return gezgin;
        gezgin = gezgin->next;
    }
    return NULL;
}

/* Dosyadan yukle */
void dosyadanYukle() {
    FILE* dosya = fopen("ilaclar.txt", "r");
    char isim[MAX_STR];
    int fiyat, stok;

    if (!dosya) {
        MessageBox(NULL, "ilaclar.txt bulunamadi!", "Hata", MB_OK | MB_ICONERROR);
        return;
    }

    while (fscanf(dosya, "%63s %d %d", isim, &fiyat, &stok) == 3) {
        sistemeIlacEkle(isim, fiyat, stok);
    }

    fclose(dosya);
}

/* Dosyaya kaydet */
void dosyayaKaydet() {
    FILE* dosya = fopen("ilaclar.txt", "w");
    int i;

    if (!dosya) {
        MessageBox(NULL, "ilaclar.txt dosyasina yazilamadi!", "Hata", MB_OK | MB_ICONERROR);
        return;
    }

    for (i = 0; i < HASH_SIZE; i++) {
        Ilac* gezgin = hashTable[i];
        while (gezgin) {
            fprintf(dosya, "%s %d %d\n", gezgin->isim, gezgin->fiyat, gezgin->stok);
            gezgin = gezgin->next;
        }
    }

    fclose(dosya);
}

/* Bilgileri kaydet */
int kullaniciBilgileriniKaydet() {
    GetWindowText(hNameEdit, aktifKullanici.adSoyad, sizeof(aktifKullanici.adSoyad));
    GetWindowText(hAddressEdit, aktifKullanici.adres, sizeof(aktifKullanici.adres));

    if (strlen(aktifKullanici.adSoyad) == 0) {
        MessageBox(NULL, "Ad soyad giriniz.", "Uyari", MB_OK | MB_ICONWARNING);
        return 0;
    }

    if (strlen(aktifKullanici.adres) == 0) {
        MessageBox(NULL, "Adres giriniz.", "Uyari", MB_OK | MB_ICONWARNING);
        return 0;
    }

    MessageBox(NULL, "Kullanici bilgileri kaydedildi.", "Bilgi", MB_OK | MB_ICONINFORMATION);
    return 1;
}

/* Sepete ekle */
int sepeteEkle(const char* ilacAdi, int adet) {
    Ilac* ilac;
    Siparis* yeniSiparis;
    char logMesaj[256];

    if (adet <= 0) return 0;

    ilac = ilacBul((char*)ilacAdi);
    if (!ilac) return -1;
    if (ilac->stok <= 0) return -2;
    if (adet > ilac->stok) return -3;

    yeniSiparis = (Siparis*)malloc(sizeof(Siparis));
    if (!yeniSiparis) return -4;

    strcpy(yeniSiparis->ilacAdi, ilac->isim);
    yeniSiparis->adet = adet;
    yeniSiparis->araToplam = adet * ilac->fiyat;
    yeniSiparis->next = NULL;

    if (sepetim.son == NULL) {
        sepetim.bas = sepetim.son = yeniSiparis;
    } else {
        sepetim.son->next = yeniSiparis;
        sepetim.son = yeniSiparis;
    }

    ilac->stok -= adet;
    genelToplam += yeniSiparis->araToplam;

    sprintf(logMesaj, "%d adet %s sepete eklendi. Kalan stok: %d",
            adet, ilac->isim, ilac->stok);
    gecmiseEkle(logMesaj);

    return 1;
}

/* Sepeti temizle */
void sepetiTemizle() {
    Siparis* sil;
    while (sepetim.bas) {
        sil = sepetim.bas;
        sepetim.bas = sepetim.bas->next;
        free(sil);
    }
    sepetim.son = NULL;
    genelToplam = 0;
}

/* Stogu geri yukle */
void sepetiIptalEtVeStoguGeriYukle() {
    Siparis* temp = sepetim.bas;

    while (temp) {
        Ilac* ilac = ilacBul(temp->ilacAdi);
        if (ilac) {
            ilac->stok += temp->adet;
        }
        temp = temp->next;
    }

    sepetiTemizle();
}

/* Sepeti guncelle */
void sepetiGuncelleGUI() {
    Siparis* temp = sepetim.bas;
    char satir[MAX_LINE];

    SendMessage(hCartList, LB_RESETCONTENT, 0, 0);

    while (temp) {
        sprintf(satir, "%s x %d = %d TL",
                temp->ilacAdi, temp->adet, temp->araToplam);
        SendMessage(hCartList, LB_ADDSTRING, 0, (LPARAM)satir);
        temp = temp->next;
    }

    sprintf(satir, "Toplam Tutar: %d TL", genelToplam);
    SetWindowText(hTotalLabel, satir);
}

/* Gecmisi guncelle */
void gecmisiGuncelleGUI() {
    Islem* gezgin = gecmisTop;

    SendMessage(hHistoryList, LB_RESETCONTENT, 0, 0);

    while (gezgin) {
        SendMessage(hHistoryList, LB_ADDSTRING, 0, (LPARAM)gezgin->logMesaji);
        gezgin = gezgin->next;
    }
}

/* Stoklari guncelle */
void stoklariGuncelleGUI() {
    int i;
    char satir[MAX_LINE];

    SendMessage(hStockList, LB_RESETCONTENT, 0, 0);

    for (i = 0; i < HASH_SIZE; i++) {
        Ilac* gezgin = hashTable[i];
        while (gezgin) {
            sprintf(satir, "%s | %d TL | Stok: %d",
                    gezgin->isim, gezgin->fiyat, gezgin->stok);
            SendMessage(hStockList, LB_ADDSTRING, 0, (LPARAM)satir);
            gezgin = gezgin->next;
        }
    }
}

/* Sonucu goster */
void ilacBilgisiGoster(const char* arama) {
    Ilac* ilac;
    char yazi[256];

    ilac = ilacBul((char*)arama);

    if (ilac) {
        sprintf(yazi, "Bulundu: %s | Fiyat: %d TL | Stok: %d",
                ilac->isim, ilac->fiyat, ilac->stok);
    } else {
        sprintf(yazi, "Ilac bulunamadi.");
    }

    SetWindowText(hResultLabel, yazi);
}

/* Arayuzu yenile */
void tumArayuzuYenile() {
    char arama[MAX_STR];

    sepetiGuncelleGUI();
    gecmisiGuncelleGUI();
    stoklariGuncelleGUI();

    GetWindowText(hSearchEdit, arama, MAX_STR);
    if (strlen(arama) > 0) {
        ilacBilgisiGoster(arama);
    } else {
        SetWindowText(hResultLabel, "Arama sonucu burada gorunecek.");
    }
}

/* Odeme yap */
void odemeYap() {
    char mesaj[512];
    char logMesaj[512];

    if (strlen(aktifKullanici.adSoyad) == 0 || strlen(aktifKullanici.adres) == 0) {
        MessageBox(NULL, "Once ad soyad ve adres bilgilerini kaydetmelisiniz.",
                   "Uyari", MB_OK | MB_ICONWARNING);
        return;
    }

    if (sepetim.bas == NULL) {
        MessageBox(NULL, "Sepet bos.", "Bilgi", MB_OK | MB_ICONINFORMATION);
        return;
    }

    sprintf(mesaj,
            "Odeme basarili!\n\nMusteri: %s\nAdres: %s\nToplam: %d TL",
            aktifKullanici.adSoyad,
            aktifKullanici.adres,
            genelToplam);

    MessageBox(NULL, mesaj, "Siparis Tamamlandi", MB_OK | MB_ICONINFORMATION);

    sprintf(logMesaj,
            "Siparis tamamlandi | Musteri: %s | Tutar: %d TL",
            aktifKullanici.adSoyad, genelToplam);
    gecmiseEkle(logMesaj);

    sepetiTemizle();
    dosyayaKaydet();
    tumArayuzuYenile();
}

/* Bellek temizle */
void bellegiTemizle() {
    int i;

    for (i = 0; i < HASH_SIZE; i++) {
        Ilac* gezgin = hashTable[i];
        while (gezgin) {
            Ilac* sil = gezgin;
            gezgin = gezgin->next;
            free(sil);
        }
        hashTable[i] = NULL;
    }

    while (gecmisTop) {
        Islem* sil = gecmisTop;
        gecmisTop = gecmisTop->next;
        free(sil);
    }

    sepetiTemizle();

    DeleteObject(hFontNormal);
    DeleteObject(hFontBold);
    DeleteObject(hFontTitle);

    DeleteObject(hBrushBg);
    DeleteObject(hBrushTitle);
    DeleteObject(hBrushButtonBlue);
    DeleteObject(hBrushButtonGreen);
    DeleteObject(hBrushButtonOrange);
    DeleteObject(hBrushButtonPurple);
}

/* Pencere islemleri */
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    char arama[MAX_STR];
    char adetText[32];
    int adet;

    switch (uMsg) {
        case WM_CREATE: {
            /* Font olustur */
            hFontNormal = CreateFont(
                18, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI"
            );

            /* Kalin font */
            hFontBold = CreateFont(
                19, 0, 0, 0, FW_BOLD, 0, 0, 0,
                DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI"
            );

            /* Baslik fontu */
            hFontTitle = CreateFont(
                30, 0, 0, 0, FW_BOLD, 0, 0, 0,
                DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI"
            );

            /* Fircalari olustur */
            hBrushBg = CreateSolidBrush(clrBg);
            hBrushTitle = CreateSolidBrush(clrTitle);
            hBrushButtonBlue = CreateSolidBrush(clrBlue);
            hBrushButtonGreen = CreateSolidBrush(clrGreen);
            hBrushButtonOrange = CreateSolidBrush(clrOrange);
            hBrushButtonPurple = CreateSolidBrush(clrPurple);

            /* Baslik */
            hTitle = CreateWindow("STATIC", "MediFast - Akilli Eczane Sistemi",
                                  WS_VISIBLE | WS_CHILD | SS_CENTER,
                                  20, 15, 980, 45,
                                  hwnd, NULL, NULL, NULL);

            /* Ad soyad etiketi */
            CreateWindow("STATIC", "Ad Soyad",
                         WS_VISIBLE | WS_CHILD,
                         40, 90, 120, 22,
                         hwnd, NULL, NULL, NULL);

            /* Ad soyad kutusu */
            hNameEdit = CreateWindow("EDIT", "",
                                     WS_VISIBLE | WS_CHILD | WS_BORDER,
                                     40, 115, 260, 30,
                                     hwnd, NULL, NULL, NULL);

            /* Adres etiketi */
            CreateWindow("STATIC", "Adres",
                         WS_VISIBLE | WS_CHILD,
                         320, 90, 120, 22,
                         hwnd, NULL, NULL, NULL);

            /* Adres kutusu */
            hAddressEdit = CreateWindow("EDIT", "",
                                        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL,
                                        320, 115, 420, 60,
                                        hwnd, NULL, NULL, NULL);

            /* Kaydet butonu */
            hBtnSaveUser = CreateWindow("BUTTON", "Bilgileri Kaydet",
                                        WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
                                        760, 120, 180, 42,
                                        hwnd, (HMENU)ID_BTN_SAVEUSER, NULL, NULL);

            /* Arama etiketi */
            CreateWindow("STATIC", "Ilac Ara",
                         WS_VISIBLE | WS_CHILD,
                         40, 205, 100, 22,
                         hwnd, NULL, NULL, NULL);

            /* Arama kutusu */
            hSearchEdit = CreateWindow("EDIT", "",
                                       WS_VISIBLE | WS_CHILD | WS_BORDER,
                                       40, 232, 220, 30,
                                       hwnd, NULL, NULL, NULL);

            /* Adet etiketi */
            CreateWindow("STATIC", "Adet",
                         WS_VISIBLE | WS_CHILD,
                         280, 205, 80, 22,
                         hwnd, NULL, NULL, NULL);

            /* Adet kutusu */
            hQtyEdit = CreateWindow("EDIT", "1",
                                    WS_VISIBLE | WS_CHILD | WS_BORDER,
                                    280, 232, 80, 30,
                                    hwnd, NULL, NULL, NULL);

            /* Ara butonu */
            hBtnSearch = CreateWindow("BUTTON", "Ara",
                                      WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
                                      380, 228, 90, 36,
                                      hwnd, (HMENU)ID_BTN_SEARCH, NULL, NULL);

            /* Sepete ekle butonu */
            hBtnAdd = CreateWindow("BUTTON", "Sepete Ekle",
                                   WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
                                   485, 228, 140, 36,
                                   hwnd, (HMENU)ID_BTN_ADD, NULL, NULL);

            /* Odeme butonu */
            hBtnPay = CreateWindow("BUTTON", "Odeme Yap",
                                   WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
                                   640, 228, 140, 36,
                                   hwnd, (HMENU)ID_BTN_PAY, NULL, NULL);

            /* Yenile butonu */
            hBtnRefresh = CreateWindow("BUTTON", "Yenile",
                                       WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
                                       795, 228, 120, 36,
                                       hwnd, (HMENU)ID_BTN_REFRESH, NULL, NULL);

            /* Sonuc yazisi */
            hResultLabel = CreateWindow("STATIC", "Arama sonucu burada gorunecek.",
                                        WS_VISIBLE | WS_CHILD,
                                        40, 275, 600, 24,
                                        hwnd, NULL, NULL, NULL);

            /* Stok etiketi */
            CreateWindow("STATIC", "Stoklar",
                         WS_VISIBLE | WS_CHILD,
                         40, 320, 150, 24,
                         hwnd, NULL, NULL, NULL);

            /* Stok listesi */
            hStockList = CreateWindow("LISTBOX", "",
                                      WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL,
                                      40, 350, 300, 270,
                                      hwnd, NULL, NULL, NULL);

            /* Sepet etiketi */
            CreateWindow("STATIC", "Sepet",
                         WS_VISIBLE | WS_CHILD,
                         370, 320, 150, 24,
                         hwnd, NULL, NULL, NULL);

            /* Sepet listesi */
            hCartList = CreateWindow("LISTBOX", "",
                                     WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL,
                                     370, 350, 260, 270,
                                     hwnd, NULL, NULL, NULL);

            /* Gecmis etiketi */
            CreateWindow("STATIC", "Islem Gecmisi",
                         WS_VISIBLE | WS_CHILD,
                         660, 320, 170, 24,
                         hwnd, NULL, NULL, NULL);

            /* Gecmis listesi */
            hHistoryList = CreateWindow("LISTBOX", "",
                                        WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL,
                                        660, 350, 300, 270,
                                        hwnd, NULL, NULL, NULL);

            /* Toplam yazisi */
            hTotalLabel = CreateWindow("STATIC", "Toplam Tutar: 0 TL",
                                       WS_VISIBLE | WS_CHILD,
                                       370, 630, 260, 28,
                                       hwnd, NULL, NULL, NULL);

            /* Font uygula */
            SendMessage(hTitle, WM_SETFONT, (WPARAM)hFontTitle, TRUE);
            SendMessage(hNameEdit, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(hAddressEdit, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(hSearchEdit, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(hQtyEdit, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(hBtnSaveUser, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessage(hBtnSearch, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessage(hBtnAdd, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessage(hBtnPay, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessage(hBtnRefresh, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessage(hResultLabel, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessage(hCartList, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(hHistoryList, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(hStockList, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(hTotalLabel, WM_SETFONT, (WPARAM)hFontBold, TRUE);

            /* Ilk yenileme */
            tumArayuzuYenile();
            return 0;
        }

        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
            HBRUSH btnBrush = hBrushButtonBlue;
            const char* text = "";
            RECT rc = dis->rcItem;

            /* Buton rengi sec */
            if (dis->CtlID == ID_BTN_SAVEUSER) {
                btnBrush = hBrushButtonPurple;
                text = "Bilgileri Kaydet";
            } else if (dis->CtlID == ID_BTN_SEARCH) {
                btnBrush = hBrushButtonBlue;
                text = "Ara";
            } else if (dis->CtlID == ID_BTN_ADD) {
                btnBrush = hBrushButtonGreen;
                text = "Sepete Ekle";
            } else if (dis->CtlID == ID_BTN_PAY) {
                btnBrush = hBrushButtonOrange;
                text = "Odeme Yap";
            } else if (dis->CtlID == ID_BTN_REFRESH) {
                btnBrush = hBrushButtonPurple;
                text = "Yenile";
            }

            /* Butonu boya */
            FillRect(dis->hDC, &rc, btnBrush);
            SetBkMode(dis->hDC, TRANSPARENT);
            SetTextColor(dis->hDC, clrWhite);
            SelectObject(dis->hDC, hFontBold);
            DrawText(dis->hDC, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            /* Fokus ciz */
            if (dis->itemState & ODS_FOCUS) {
                DrawFocusRect(dis->hDC, &rc);
            }
            return TRUE;
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            HWND hCtrl = (HWND)lParam;

            /* Baslik rengi */
            if (hCtrl == hTitle) {
                SetTextColor(hdcStatic, clrWhite);
                SetBkColor(hdcStatic, clrTitle);
                return (INT_PTR)hBrushTitle;
            } else {
                SetTextColor(hdcStatic, clrTextDark);
                SetBkColor(hdcStatic, clrBg);
                return (INT_PTR)hBrushBg;
            }
        }

        case WM_CTLCOLOREDIT: {
            HDC hdcEdit = (HDC)wParam;

            /* Edit rengi */
            SetTextColor(hdcEdit, clrTextDark);
            SetBkColor(hdcEdit, RGB(255, 255, 255));
            return (INT_PTR)GetStockObject(WHITE_BRUSH);
        }

        case WM_CTLCOLORLISTBOX: {
            HDC hdcList = (HDC)wParam;

            /* Liste rengi */
            SetTextColor(hdcList, clrTextDark);
            SetBkColor(hdcList, RGB(255, 255, 255));
            return (INT_PTR)GetStockObject(WHITE_BRUSH);
        }

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_BTN_SAVEUSER:
                    /* Kullaniciyi kaydet */
                    kullaniciBilgileriniKaydet();
                    return 0;

                case ID_BTN_SEARCH:
                    /* Ilac ara */
                    GetWindowText(hSearchEdit, arama, MAX_STR);
                    if (strlen(arama) == 0) {
                        MessageBox(hwnd, "Ilac adi giriniz.", "Uyari", MB_OK | MB_ICONWARNING);
                        return 0;
                    }
                    ilacBilgisiGoster(arama);
                    return 0;

                case ID_BTN_ADD:
                    /* Sepete ekle */
                    GetWindowText(hSearchEdit, arama, MAX_STR);
                    GetWindowText(hQtyEdit, adetText, 31);

                    if (strlen(arama) == 0) {
                        MessageBox(hwnd, "Ilac adi giriniz.", "Uyari", MB_OK | MB_ICONWARNING);
                        return 0;
                    }

                    adet = atoi(adetText);
                    if (adet <= 0) {
                        MessageBox(hwnd, "Gecerli adet giriniz.", "Uyari", MB_OK | MB_ICONWARNING);
                        return 0;
                    }

                    {
                        int sonuc = sepeteEkle(arama, adet);

                        if (sonuc == 1) {
                            MessageBox(hwnd, "Urun sepete eklendi.", "Bilgi", MB_OK | MB_ICONINFORMATION);
                            dosyayaKaydet();
                            tumArayuzuYenile();
                        } else if (sonuc == -1) {
                            MessageBox(hwnd, "Ilac bulunamadi.", "Hata", MB_OK | MB_ICONERROR);
                        } else if (sonuc == -2) {
                            MessageBox(hwnd, "Stokta kalmadi.", "Hata", MB_OK | MB_ICONERROR);
                        } else if (sonuc == -3) {
                            MessageBox(hwnd, "Yetersiz stok.", "Hata", MB_OK | MB_ICONERROR);
                        } else {
                            MessageBox(hwnd, "Bir hata olustu.", "Hata", MB_OK | MB_ICONERROR);
                        }
                    }
                    return 0;

                case ID_BTN_PAY:
                    /* Odeme yap */
                    odemeYap();
                    return 0;

                case ID_BTN_REFRESH:
                    /* Her seyi sifirla */
                    sepetiIptalEtVeStoguGeriYukle();
                    gecmisiTemizle();

                    SetWindowText(hSearchEdit, "");
                    SetWindowText(hQtyEdit, "1");
                    SetWindowText(hResultLabel, "Arama sonucu burada gorunecek.");

                    dosyayaKaydet();
                    tumArayuzuYenile();

                    MessageBox(hwnd, "Tum islemler temizlendi.", "Bilgi", MB_OK | MB_ICONINFORMATION);
                    return 0;
            }
            return 0;

        case WM_ERASEBKGND: {
            RECT rc;
            HDC hdc = (HDC)wParam;

            /* Arka plani boya */
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, hBrushBg);
            return 1;
        }

        case WM_DESTROY:
            /* Cikarken kaydet */
            dosyayaKaydet();
            bellegiTemizle();
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/* Program baslangici */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "MediFastWindowClass";
    WNDCLASS wc;
    HWND hwnd;
    MSG msg;

    /* Sinifi hazirla */
    memset(&wc, 0, sizeof(WNDCLASS));
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    /* Sinifi kaydet */
    RegisterClass(&wc);

    /* Veriyi yukle */
    dosyadanYukle();

    /* Pencereyi olustur */
    hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "MediFast - Akilli Eczane Sistemi",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1020, 740,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) return 0;

    /* Pencereyi goster */
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    /* Mesaj dongusu */
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}