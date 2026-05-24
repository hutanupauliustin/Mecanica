# Simulator Mecanic 2D (Rigid Body Physics Engine)



Un motor fizic bidimensional scris de la zero în **C++17**, capabil să simuleze sisteme complexe de corpuri rigide și constrângeri cinematice (articulații, încastrări). Proiectul folosește formularea analitică Lagrange de speța întâi cu multiplicatori direcți pentru o simulare stabilă și precisă.



<table>
  <tr>
    <td align="center">
      <img src="doc/gifuri2/demolare.gif" width="200">
    </td>
    <td align="center">
      <img src="doc/gifuri2/newton.gif" width="200" >
    </td>
    <td align="center">
      <img src="doc/gifuri2/pendul.gif" width="200" >
    </td>
  </tr>
</table>



## Funcționalități Principale



* **Dinamica Corpurilor Rigide:** Simulare cu integrare numerică Runge-Kutta de ordinul 4 (RK4).

* **Solver de Constrângeri Stabil:** Rezolvarea sistemului de ecuații simetric pozitiv definit folosind factorizarea **Cholesky**.

* **Coliziuni și Contacte (SAT):** Detecție exactă a coliziunilor bazată pe Teorema Axelor de Separare (Separating Axis Theorem) și extragerea punctelor de contact prin algoritmi de decupare (clipping).

* **Rezolvarea Impulsurilor:** Răspuns la coliziuni calculat la nivel de viteză pentru menținerea intactă a constrângerilor post-impact.

* **Interfață Interactivă:** Interfață vizuală în timp real (Play/Pause, Slider de Viteză, Editare peisaj) construită cu Dear ImGui și grafice de eforturi în timp real utilizând ImPlot.



## Tehnologii Utilizate



* **Limbaj:** C++17

* **Grafică / Interfață:** OpenGL 3+, GLFW, Dear ImGui, ImPlot

* **Matematică / Fizică:** Implementarea propriilor biblioteci de calcul matriceal si de solver-e pentru sistemele de ecuatii.

* **Build System:** CMake



## Cum se compilează și rulează



Acest proiect folosește **CMake** pentru configurare și se compilează optim în modul `Release` pentru performanță maximă (60 FPS).



### Pre-rechizite

* Un compilator C++17 (ex: GCC / MinGW via MSYS2 pe Windows).

* CMake instalat.



### Pași de build

```# 1. Clonează repository-ul

git clone [https://github.com/hutanupauliustin/Mecanica.git](https://github.com/hutanupauliustin/Mecanica.git)

cd Mecanica



# 2. Generează fișierele de build

mkdir build

cd build

cmake -DCMAKE_BUILD_TYPE=Release ..



# 3. Compilează

cmake --build .



# 4. Rulează executabilul rezultat

./simulator

