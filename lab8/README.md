# Zadania - Zestaw 8
## Filtrowanie obrazów
Jedną z najprostszych operacji jaką można wykonać na obrazie jest operacja filtrowania (splotu). Operacja ta przyjmuje na wejściu dwie macierze:

- Macierz ![eq1](https://github.com/pmaczuga/sysopy/blob/master/lab8/readme_files/eq1.gif) reprezentującą obraz. Dla uproszczenia rozważamy jedynie obrazy w 256 odcieniach szarości. Każdy element macierzy ![eq2](https://github.com/pmaczuga/sysopy/blob/master/lab8/readme_files/eq2.gif) jest więc liczbą całkowitą z zakresu 0 do 255.
- Macierz ![eq3](https://github.com/pmaczuga/sysopy/blob/master/lab8/readme_files/eq3.gif) reprezentującą filtr. Elementami tej macierzy są liczby zmiennoprzecinkowe. Dla uproszczenia zakładamy, że elementy macierzy ![eq4](https://github.com/pmaczuga/sysopy/blob/master/lab8/readme_files/eq4.gif) sumują się do jedności: ![eq5](https://github.com/pmaczuga/sysopy/blob/master/lab8/readme_files/eq5.gif).

Operacja filtrowania tworzy nowy obraz ![eqJ](https://github.com/pmaczuga/sysopy/blob/master/lab8/readme_files/eqJ.gif), którego piksele mają wartość:

![eq6](https://github.com/pmaczuga/sysopy/blob/master/lab8/readme_files/eq6.gif),

![eq7](https://github.com/pmaczuga/sysopy/blob/master/lab8/readme_files/eq7.gif).

Operacja  ![eq8](https://github.com/pmaczuga/sysopy/blob/master/lab8/readme_files/eq8.gif)  oznacza zaokrąglenie do najbliższej liczby całkowitej a ![eqCeil](https://github.com/pmaczuga/sysopy/blob/master/lab8/readme_files/eqCeil.gif) zaokrąglenie w górę do najbliższej liczby całkowitej.

Zwróć uwagę, że w powyższym opisie przyjęto matematyczną konwencję indeksowania elementów macierzy - od indeksu 1.

### Zadanie 1
Napisz program, który wykonuje wielowątkową operację filtrowania obrazu. Program przyjmuje w argumentach wywołania:

1. liczbę wątków,
2. nazwę pliku z wejściowym obrazem,
3. nazwę pliku z definicją filtru,
4. nazwę pliku wynikowego.

Po wczytaniu danych (wejściowy obraz i definicja filtru) wątek główny tworzy tyle nowych wątków, ile zażądano w argumencie wywołania. Utworzone wątki równolegle generują wyjściowy (filtrowany) obraz. Każdy wątek odpowiada za wygenerowanie fragmentu wyniku - np. generuje pionowy pasek o szerokości ![eq9](https://github.com/pmaczuga/sysopy/blob/master/lab8/readme_files/eq9.gif) pikseli, gdzie ![eq10](https://github.com/pmaczuga/sysopy/blob/master/lab8/readme_files/eq10.gif) to szerokość wyjściowego obrazu a ![eq11](https://github.com/pmaczuga/sysopy/blob/master/lab8/readme_files/eq11.gif) to liczba wątków. Wątek główny czeka na zakończenie pracy przez wątki wykonujące operację filtrowania. Następnie zapisuje powstały obraz do pliku wynikowego. Dodatkowo wątek główny mierzy czas rzeczywisty operacji filtrowania i wypisuje go na ekranie. W mierzonym czasie należy uwzględnić narzut związany z utworzeniem i zakończeniem wątków (ale bez czasu operacji wejścia/wyjścia).

Wykonaj pomiary czasu operacji filtrowania dla obrazu o rozmiarze kilkaset na kilkaset pikseli i kilku filtrów (można wykorzystać losowe macierze filtrów). Testy przeprowadź dla 1, 2, 4, i 8 wątków. Rozmiar filtrów dobierz w zakresie ![eq12](https://github.com/pmaczuga/sysopy/blob/master/lab8/readme_files/eq12.gif), tak aby uwidocznić wpływ liczby wątków na czas operacji filtrowania . Wyniki zamieść w pliku ```Times.txt``` i dołącz do archiwum z rozwiązaniem zadania.
### Format wejścia-wyjścia
Program powinien odczytywać i zapisywać obrazy w formacie ASCII PGM (Portable Gray Map). Pliki w tym formacie mają nagłówek postaci:

```
P2
W H
M
...
```

gdzie: ```W``` to szerokość obrazu w pikselach, ```H``` to wysokość obrazu w pikselach a M to maksymalna wartość piksela. Zakładamy, że obsługujemy jedynie obrazy w 256 odcieniach szarości: od 0 do 255 (a więc ![eq13](https://github.com/pmaczuga/sysopy/blob/master/lab8/readme_files/eq13.gif)). Po nagłówku, w pliku powinno być zapisanych ```W*H``` liczb całkowitych reprezentujących wartości kolejnych pikseli. Liczby rozdzielone są białymi znakami (np. spacją). Piksele odczytywane są wierszami, w kolejności od lewego górnego do prawego dolnego rogu obrazu.

Przykładowe obrazy w formacie ASCII PGM (jak również opis formatu) można znaleźć pod adresem: http://people.sc.fsu.edu/~jburkardt/data/pgma/pgma.html

W pierwszej linii pliku z definicją filtru powinna znajdować się liczba całkowita c określająca rozmiar filtru. Dalej, plik powinien zawierać ![eq14](https://github.com/pmaczuga/sysopy/blob/master/lab8/readme_files/eq14.gif) liczb zmiennoprzecinkowych określających wartości elementów filtru (w kolejności wierszy, od elementu ![eq15](https://github.com/pmaczuga/sysopy/blob/master/lab8/readme_files/eq15.gif) do elementu ![eq16](https://github.com/pmaczuga/sysopy/blob/master/lab8/readme_files/eq16.gif)).
