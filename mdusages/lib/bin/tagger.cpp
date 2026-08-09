#include <iostream>
#include <list>
#include <iterator>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <windows.h>

// Envoie une chaîne UTF-8 directement dans le presse-papiers de Windows
void copierDansPressePapier(const std::string& texte) {
    // 1. Convertir le texte UTF-8 en UTF-16 (requis par le presse-papiers Windows pour les accents)
    int wsize = MultiByteToWideChar(CP_UTF8, 0, texte.c_str(), -1, NULL, 0);
    std::wstring wtexte(wsize, 0);
    MultiByteToWideChar(CP_UTF8, 0, texte.c_str(), -1, &wtexte[0], wsize);

    // 2. Allouer de la mémoire globale pour le presse-papiers
    size_t tailleOctets = wsize * sizeof(wchar_t);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, tailleOctets);
    if (!hMem) return;

    // 3. Copier les données dans cette mémoire
    memcpy(GlobalLock(hMem), wtexte.c_str(), tailleOctets);
    GlobalUnlock(hMem);

    // 4. Ouvrir le presse-papiers et y injecter le texte
    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        SetClipboardData(CF_UNICODETEXT, hMem);
        CloseClipboard();
    } else {
        GlobalFree(hMem); // Libérer la mémoire si l'ouverture a échoué
    }
}

std::string getArgv1InUtf8() {
    int argc;
    wchar_t** argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argvW || argc < 2) return "";

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, argvW[1], -1, NULL, 0, NULL, NULL);
    std::string strTo(size_needed - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, argvW[1], -1, &strTo[0], size_needed, NULL, NULL);
    
    LocalFree(argvW);
    return strTo;
}

bool isLetter(char32_t c) {
    if ((c >= 0x0041 && c <= 0x005A) || (c >= 0x0061 && c <= 0x007A)) {
        return true; 
    }
    if ((c >= 0x00C0 && c <= 0x00FF) || (c >= 0x0100 && c <= 0x017F)) {
        return true; 
    }
    return false; 
}

std::u32string utf8_to_u32(const std::string& str) {
    if (str.empty()) return U"";
    int wsize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    std::wstring wstr(wsize - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], wsize);
    return std::u32string(wstr.begin(), wstr.end());
}

// Convertit une chaîne UTF-32 (std::u32string) vers UTF-8 (std::string) pour l'affichage
std::string u32_to_utf8(const std::u32string& u32str) {
    if (u32str.empty()) return "";
    std::wstring wstr(u32str.begin(), u32str.end());
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    std::string strTo(size_needed - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "./tagger <## TARGET TO TAG>\n into : [TARGET TO TAG](target-to-tag) for link to tag in a markdown" << std::endl;
        return 1;
    }

    std::system("chcp 65001 > nul");
    
    std::string str = getArgv1InUtf8();

    if (str.empty() || str.size() < 4) {
        std::cout << "./tagger <## TARGET TO TAG>\n into : [TARGET TO TAG](target-to-tag) for link to tag in a markdown" << std::endl;
        return 1;
    }

    // Extraction du titre propre (on enlève les symboles de départ comme "## ")
    std::string head = str.substr(0); 
    std::u32string headU32 = utf8_to_u32(head);

    std::list<char32_t> tag_list;

    for (size_t i = 0; i < headU32.size(); i++)
    {
        char32_t c = headU32[i];
        
        if (isLetter(c)) {
            // Convertir en minuscule pour le format de l'ancre du lien Markdown
            if (c >= 0x0041 && c <= 0x005A) {
                c += 32; // A-Z -> a-z
            } else if (c >= 0x00C0 && c <= 0x00DC) {
                c += 32; // Accents Majuscules -> Accents Minuscules
            }
            tag_list.push_back(c);
        } 
        else if (c == U' ' || c == U'-') {
            // Remplacer les espaces par des tirets pour le format d'ancre Markdown
            // Éviter de mettre plusieurs tirets d'affilée
            if (!tag_list.empty() && tag_list.back() != U'-') {
                tag_list.push_back(U'-');
            }
        }
    }

    // Supprimer un éventuel tiret traînant à la fin de la liste
    if (!tag_list.empty() && tag_list.back() == U'-') {
        tag_list.pop_back();
    }

    // Reconstruction de la chaîne du tag (ancre)
    std::u32string tagU32(tag_list.begin(), tag_list.end());
    std::string tagUtf8 = u32_to_utf8(tagU32);

     // 1. On prépare la chaîne finale du lien Markdown
    std::string resultatMarkdown = "[" + head + "](#" + tagUtf8 + ")";

    // 2. On l'affiche toujours dans la console pour contrôle
    std::cout << resultatMarkdown << "   copie success !" << std::endl;

    copierDansPressePapier(resultatMarkdown);

    return 0;
}
