#include "Engine/Utils/StringUtils.h"
#include <sstream>


namespace string_utils
{
    std::vector<std::string> split_text(const std::string& message, size_t taille_max)
    {
        // On déclare la variable de retour
        std::vector<std::string> res;

        // Si la taille max est 0 on return un vecteur de string vide
        if (taille_max == 0) return res;

        // On déclare et initialise de quoi parcourir le message
        size_t start = 0;
        const std::string& s = message;

        // On parcourt le message
        while (start <= s.size()) {
            // On détecte les paragraphes
            size_t pos = s.find('\n', start);
            std::string paragraph = (pos == std::string::npos) ? s.substr(start) : s.substr(start, pos - start);

            // Utilise un istringstream pour itérer par mot
            std::istringstream iss(paragraph);
            std::string word;
            std::string line;
            while (iss >> word) {
                if (line.empty()) {
                    if (word.size() <= taille_max) {
                        line = word;
                    } else {
                        // Mot trop long -> on le coupe
                        size_t p = 0;
                        while (p < word.size()) {
                            size_t len = std::min(taille_max, word.size() - p);
                            res.push_back(word.substr(p, len));
                            p += len;
                        }
                    }
                } else {
                    if (line.size() + 1 + word.size() <= taille_max) {
                        line += " " + word;
                    } else {
                        res.push_back(line);
                        if (word.size() <= taille_max) {
                            line = word;
                        } else {
                            size_t p = 0;
                            while (p < word.size()) {
                                size_t len = std::min(taille_max, word.size() - p);
                                res.push_back(word.substr(p, len));
                                p += len;
                            }
                            line.clear();
                        }
                    }
                }
            }

            if (!line.empty()) {
                res.push_back(line);
            }

            if (pos == std::string::npos) break;
            // Conserver un saut de ligne explicite (ligne vide)
            res.push_back(std::string());
            start = pos + 1;
        }

        if (res.empty()) res.push_back(std::string());
        return res;
    }
}