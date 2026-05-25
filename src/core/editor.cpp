#include "editor.h"
#include <cmath>
#include <algorithm>
#include <cstdio>
#include "instrument.h"
#include <filesystem>
#include "portable-file-dialogs.h"

editor::editor()
{
    mod_curent = MOD_RULARE;
    flag.mod_vizualizare = 0;
    cadru_activ = 0;
    mouse_x = 0.0f;
    mouse_y = 0.0f;
    frameCount = 0;

    elementeSelectate.resize(0);

    elementeUI.resize(2);

    valoriSimulate.reserve(1000);

    fisier_export.open(nume_fisier_export);
    if (fisier_export.is_open())
    {
        fisier_export << "Timp,ID_Corp,PozitieX,PozitieY,UnghiPhi,VitezaX,VitezaY,VitezaOmega,AcceleratieX,AcceleratieY,AcceleratieEpsilon\n";
    }

    fisier_export_legaturi.open(nume_fisier_export_legaturi);
    if (fisier_export_legaturi.is_open())
    {
        // Am adaugat FortaX_N si FortaY_N in capul de tabel
        fisier_export_legaturi << "Timp,ID_Legatura,FortaModul_N,FortaX_N,FortaY_N,Moment_Nm\n";
    }
    instrumentCurent = std::make_unique<InstrumentSelectie>();
}

editor::~editor()
{
    if (fisier_export.is_open())
    {
        fisier_export.close();
    }
    if (fisier_export_legaturi.is_open())
    {
        fisier_export_legaturi.close();
    }

    std::remove(nume_fisier_export.c_str());
    std::remove(nume_fisier_export_legaturi.c_str());
}

void editor::mutaCorp(sistem &S, int idCorp, float offsetX, float offsetY)
{

    S.corpuri[idCorp].pozitie.x = this->mouse_x + offsetX;
    S.corpuri[idCorp].pozitie.y = this->mouse_y + offsetY;
}

ObiectSelectat editor::gasesteObiectSubMouse(sistem &S)
{

    ObiectSelectat rezultat = {TIP_CORP, -1}; // Initializam cu "nimic gasit"

    // ==========================================
    // PRIORITATEA 1: LEGATURI
    // ==========================================
    int id_legatura = -1;
    float distanta_minima_leg = 0.25f;

    for (int i = 0; i < (int)S.legaturi.size(); i++)
    {
        if (!S.legaturi[i]->activ)
            continue;

        vec2 pA = S.corpuri[S.legaturi[i]->contorCorpA].localToGlobal(S.legaturi[i]->getPozitieA());
        vec2 pB = S.corpuri[S.legaturi[i]->contorCorpB].localToGlobal(S.legaturi[i]->getPozitieB());

        vec2 AB = pB - pA;
        vec2 AM = vec2(this->mouse_x, this->mouse_y) - pA;
        float lungime_sq = AB.x * AB.x + AB.y * AB.y;

        float t = std::max(0.0f, std::min(1.0f, (AM.x * AB.x + AM.y * AB.y) / (lungime_sq + 0.0001f)));
        vec2 proiectie = pA + AB * t;
        float dist = (vec2(this->mouse_x, this->mouse_y) - proiectie).modul();

        if (dist < distanta_minima_leg)
        {
            distanta_minima_leg = dist;
            id_legatura = i;
        }
    }

    if (id_legatura != -1)
    {
        rezultat.tip = TIP_LEGATURA;
        rezultat.id = id_legatura;
        return rezultat;
    }

    // Aici vei adauga Prioritatea 2: Generatori forte etc...

    // ==========================================
    // PRIORITATEA 3: CORPURI
    // ==========================================
    int id_corp = -1;
    int cadru_max = -999999;
    float min_dist_corp = 999999.9f;

    for (int k = S.corpuri.size() - 1; k >= 1; k--)
    {
        if (!S.corpuri[k].activ || S.corpuri[k].collider.obiectVirtual)
            continue;
        if (this->mod_curent == MOD_EDITARE && S.corpuri[k].collider.cadru != this->cadru_activ)
            continue;

        rigid &target = S.corpuri[k];
        float dx = this->mouse_x - target.pozitie.x;
        float dy = this->mouse_y - target.pozitie.y;
        vec2 punct_local = target.globalToLocal(vec2(this->mouse_x, this->mouse_y));

        bool lovit = false;
        float cur_dist = 999999.0f;

        if (target.collider.tip == DREPTUNGHI)
        {
            float hw = target.collider.dimensiune1 / 2.0f;
            float hh = target.collider.dimensiune2 / 2.0f;
            float halo = (this->mod_curent == MOD_EDITARE) ? std::max(0.15f, std::min(hw, hh) * 0.5f) : 0.0f;

            if (std::abs(punct_local.x) <= hw + halo && std::abs(punct_local.y) <= hh + halo)
            {
                lovit = true;
                cur_dist = std::sqrt(dx * dx + dy * dy);
            }
        }
        else if (target.collider.tip == CERC)
        {
            float R = target.collider.dimensiune1;
            float halo = (this->mod_curent == MOD_EDITARE) ? std::max(0.15f, R * 0.3f) : 0.0f;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist <= R + halo)
            {
                lovit = true;
                cur_dist = dist;
            }
        }

        if (lovit)
        {
            int cadru_curent = target.collider.cadru;
            if (cadru_curent > cadru_max || (cadru_curent == cadru_max && cur_dist < min_dist_corp - 0.1f))
            {
                cadru_max = cadru_curent;
                min_dist_corp = cur_dist;
                id_corp = k;
            }
        }
    }

    if (id_corp != -1)
    {
        rezultat.tip = TIP_CORP;
        rezultat.id = id_corp;
        return rezultat;
    }

    return rezultat;
}

void editor::sincronizeazaMemorie(sistem &S)
{
    if (valoriSimulate.size() < S.corpuri.size())
    {
        size_t old_size = valoriSimulate.size();
        valoriSimulate.resize(S.corpuri.size());

        for (size_t i = old_size; i < valoriSimulate.size(); i++)
        {
            valoriSimulate[i].timpAfisat.reserve(valoriSimulate[i].capacitate_maxima);
            for (int axa = 0; axa < TOTAL_PARAMETRII; axa++)
            {
                valoriSimulate[i].axe[axa].reserve(valoriSimulate[i].capacitate_maxima);
            }
        }
    }

    // Curățăm istoricul corpurilor inactive pentru a preveni amestecarea graficelor dacă indexul este refolosit
    for (size_t i = 0; i < S.corpuri.size(); i++)
    {
        if (!S.corpuri[i].activ && valoriSimulate.size() > i)
        {

            valoriSimulate[i].timpAfisat.clear();
            for (int axa = 0; axa < TOTAL_PARAMETRII; axa++)
            {
                valoriSimulate[i].axe[axa].clear();
            }
            valoriSimulate[i].offset = 0;
        }
    }

    if (valoriLegaturi.size() < S.legaturi.size())
    {
        size_t old_size = valoriLegaturi.size();
        valoriLegaturi.resize(S.legaturi.size());

        for (size_t i = old_size; i < valoriLegaturi.size(); i++)
        {
            valoriLegaturi[i].timpAfisat.reserve(valoriLegaturi[i].capacitate_maxima);
            valoriLegaturi[i].fortaModul.reserve(valoriLegaturi[i].capacitate_maxima);
            valoriLegaturi[i].fortaX.reserve(valoriLegaturi[i].capacitate_maxima);
            valoriLegaturi[i].fortaY.reserve(valoriLegaturi[i].capacitate_maxima);
            valoriLegaturi[i].moment.reserve(valoriLegaturi[i].capacitate_maxima);
        }
    }

    for (size_t i = 0; i < S.legaturi.size(); i++)
    {
        if (!S.legaturi[i]->activ && valoriLegaturi.size() > i)
        {
            valoriLegaturi[i].timpAfisat.clear();
            valoriLegaturi[i].fortaModul.clear();
            valoriLegaturi[i].fortaX.clear();
            valoriLegaturi[i].fortaY.clear();
            valoriLegaturi[i].moment.clear();
            valoriLegaturi[i].offset = 0;
        }
    }
}

void editor::incarcaDatePentruGrafic(sistem &S)
{
    std::string csv_buffer;
    csv_buffer.reserve(S.corpuri.size() * 100);

    if (valoriSimulate.size() < S.corpuri.size()) {
        valoriSimulate.resize(S.corpuri.size());
    }

    for (size_t i = 0; i < S.corpuri.size(); i++)
    {

        if (!S.corpuri[i].activ || S.corpuri[i].M > 1e10f)
            continue;

        IstoricCorp &istoric = valoriSimulate[i];

        if ((int)istoric.timpAfisat.size() < istoric.capacitate_maxima)
        {
            istoric.timpAfisat.push_back(S.t);
            istoric.axe[POZITIE_X].push_back(S.corpuri[i].pozitie.x);
            istoric.axe[POZITIE_Y].push_back(S.corpuri[i].pozitie.y);
            istoric.axe[POZITIE_PHI].push_back(S.corpuri[i].phi);
            istoric.axe[VITEZA_X].push_back(S.corpuri[i].viteza.x);
            istoric.axe[VITEZA_Y].push_back(S.corpuri[i].viteza.y);
            istoric.axe[VITEZA_OMEGA].push_back(S.corpuri[i].omega);
            istoric.axe[ACCELERATIE_X].push_back(S.corpuri[i].forte_desen.acc_cadru.x);
            istoric.axe[ACCELERATIE_Y].push_back(S.corpuri[i].forte_desen.acc_cadru.y);
            istoric.axe[ACCELERATIE_EPSILON].push_back(S.corpuri[i].forte_desen.eps_cadru);
        }
        else
        {
            istoric.timpAfisat[istoric.offset] = S.t;
            istoric.axe[POZITIE_X][istoric.offset] = S.corpuri[i].pozitie.x;
            istoric.axe[POZITIE_Y][istoric.offset] = S.corpuri[i].pozitie.y;
            istoric.axe[POZITIE_PHI][istoric.offset] = S.corpuri[i].phi;
            istoric.axe[VITEZA_X][istoric.offset] = S.corpuri[i].viteza.x;
            istoric.axe[VITEZA_Y][istoric.offset] = S.corpuri[i].viteza.y;
            istoric.axe[VITEZA_OMEGA][istoric.offset] = S.corpuri[i].omega;
            istoric.axe[ACCELERATIE_X][istoric.offset] = S.corpuri[i].forte_desen.acc_cadru.x;
            istoric.axe[ACCELERATIE_Y][istoric.offset] = S.corpuri[i].forte_desen.acc_cadru.y;
            istoric.axe[ACCELERATIE_EPSILON][istoric.offset] = S.corpuri[i].forte_desen.eps_cadru;

            istoric.offset = (istoric.offset + 1) % istoric.capacitate_maxima;
        }

        char linie_csv[256];
        std::snprintf(linie_csv, sizeof(linie_csv), "%.4f,%d,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n",
                      S.t, (int)i, S.corpuri[i].pozitie.x, S.corpuri[i].pozitie.y, S.corpuri[i].phi,
                      S.corpuri[i].viteza.x, S.corpuri[i].viteza.y, S.corpuri[i].omega,
                      S.corpuri[i].forte_desen.acc_cadru.x, S.corpuri[i].forte_desen.acc_cadru.y, S.corpuri[i].forte_desen.eps_cadru);
        csv_buffer += linie_csv;
    }

    if (fisier_export.is_open() && !csv_buffer.empty())
    {
        fisier_export << csv_buffer;
    }

    std::string csv_legaturi;
    csv_legaturi.reserve(S.legaturi.size() * 100);

    if (valoriLegaturi.size() < S.legaturi.size()) {
        valoriLegaturi.resize(S.legaturi.size());
    }

    for (size_t i = 0; i < S.legaturi.size(); i++)
    {
        if (!S.legaturi[i]->activ)
            continue;

        IstoricLegatura &ist = valoriLegaturi[i];

        // Extragem valorile din torsorul de reacțiune existent în clasa legatura
        float fX = S.legaturi[i]->fortaReactiune.forta.x;
        float fY = S.legaturi[i]->fortaReactiune.forta.y;
        float modulF = S.legaturi[i]->fortaReactiune.forta.modul();
        float momentF = S.legaturi[i]->fortaReactiune.moment;

        if ((int)ist.timpAfisat.size() < ist.capacitate_maxima)
        {
            ist.timpAfisat.push_back(S.t);
            ist.fortaModul.push_back(modulF);
            ist.fortaX.push_back(fX);
            ist.fortaY.push_back(fY);
            ist.moment.push_back(momentF);
        }
        else
        {
            ist.timpAfisat[ist.offset] = S.t;
            ist.fortaModul[ist.offset] = modulF;
            ist.fortaX[ist.offset] = fX;
            ist.fortaY[ist.offset] = fY;
            ist.moment[ist.offset] = momentF;
            ist.offset = (ist.offset + 1) % ist.capacitate_maxima;
        }

        char linie_leg[256]; // Mărit pentru a încăpea 6 numere
        std::snprintf(linie_leg, sizeof(linie_leg), "%.4f,%d,%.4f,%.4f,%.4f,%.4f\n",
                      S.t, (int)i, modulF, fX, fY, momentF);
        csv_legaturi += linie_leg;
    }

    if (fisier_export_legaturi.is_open() && !csv_legaturi.empty())
    {
        fisier_export_legaturi << csv_legaturi;
    }
}

void editor::updateMousePosition() {
    double mx, my;
    glfwGetCursorPos(this->window, &mx, &my);

    // --- LOGICA NOUA PENTRU PANNING CAMERA ---
    static double last_mx = mx, last_my = my; // Tine minte pozitia din cadrul anterior
    
    if (glfwGetMouseButton(this->window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
        int w, h;
        glfwGetWindowSize(this->window, &w, &h);
        
        // Transformam pixelii parcursi de mouse in unitati normalizate (NDC)
        float dx = (mx - last_mx) * 2.0f / (w ? w : 1);
        float dy = (my - last_my) * 2.0f / (h ? h : 1);

        // Mutam camera invers fata de cum tragem mouse-ul (y e inversat pe ecran)
        this->camera.x -= dx * this->camera.zoom * this->camera.aspect_ratio;
        this->camera.y += dy * this->camera.zoom; 
    }
    
    last_mx = mx;
    last_my = my;
    // -----------------------------------------

    int width, height;
    glfwGetWindowSize(this->window, &width, &height);
    
    if (width == 0) width = 1;
    if (height == 0) height = 1;

    // ... restul functiei tale ramane la fel ...
    float ndcX = (2.0f * (float)mx) / width - 1.0f;
    float ndcY = 1.0f - (2.0f * (float)my) / height; 
    
    this->mouse_x = ndcX * this->camera.zoom * this->camera.aspect_ratio + camera.x;
    this->mouse_y = ndcY * this->camera.zoom + this->camera.y;    
}

void editor::proceseazaClick(sistem &S, int buton, int actiune)
{
    if (!instrumentCurent)
        return;

    if (buton == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (actiune == GLFW_PRESS)
        {
            instrumentCurent->clickStanga(S, *this, mouse_x, mouse_y);
        }
        else if (actiune == GLFW_RELEASE)
        {
            instrumentCurent->eliberareClickStanga(S, *this);
        }
    }
    else if (buton == GLFW_MOUSE_BUTTON_RIGHT && actiune == GLFW_PRESS)
    {
        instrumentCurent->clickDreapta(S, *this);
    }
}

void editor::proceseazaMiscareMouse(sistem &S)
{
    if (instrumentCurent)
    {
        instrumentCurent->miscareMouse(S, *this, mouse_x, mouse_y);
    }
}

void editor::schimbaInstrumentCurent(InstrumentEditor *instrumentNou)
{
    instrumentCurent.reset(instrumentNou);

    for (auto &f : elementeUI)
    {
        f.activa = false;
    }
}

void editor::salveazaLogCorpuri() {
    if (fisier_export.is_open()) {
        fisier_export.flush();
    }

    auto dest = pfd::save_file("Salveaza log corpuri", "", {"Fisier CSV", "*.csv"}, pfd::opt::force_overwrite);
    if (!dest.result().empty()) {
        try {
            std::filesystem::copy_file(nume_fisier_export, dest.result(), std::filesystem::copy_options::overwrite_existing);
        } catch (const std::filesystem::filesystem_error& e) {
            pfd::message("Eroare Salvare", std::string("Nu s-a putut salva fisierul de log pentru corpuri:\n") + e.what(), pfd::choice::ok, pfd::icon::error);
        }
    }
}

void editor::salveazaLogLegaturi() {
    if (fisier_export_legaturi.is_open()) {
        fisier_export_legaturi.flush();
    }

    auto dest = pfd::save_file("Salveaza log legaturi", "", {"Fisier CSV", "*.csv"}, pfd::opt::force_overwrite);
    if (!dest.result().empty()) {
        try {
            std::filesystem::copy_file(nume_fisier_export_legaturi, dest.result(), std::filesystem::copy_options::overwrite_existing);
        } catch (const std::filesystem::filesystem_error& e) {
            pfd::message("Eroare Salvare", std::string("Nu s-a putut salva fisierul de log pentru legaturi:\n") + e.what(), pfd::choice::ok, pfd::icon::error);
        }
    }
}