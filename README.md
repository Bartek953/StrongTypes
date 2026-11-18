# StrongTypes
Project from C++ course.

Podział pracy:
Zadanie bardzo naturalnie dzieli się na dwie niezależne części: **rdzeń i metaprogramowanie (silnik typów i zależności)** oraz **implementację umiejętności wbudowanych (CRTP mixins i operatory)**. Taki podział minimalizuje konflikty w kodzie i pozwala na niemal równoległą pracę od samego początku.

**Krok 0: Wspólny start (ok. 15–30 minut)**
Zanim zaczniecie pisać osobno, stwórzcie wspólnie szkielet pliku `strong.h`:

* Ustalcie nazwę wewnętrznej przestrzeni nazw (np. `strong_detail`) oraz dodajcie *header guard*.


* Zdefiniujcie atrapę (mock) klasy `strong`, `skill_access` oraz cechy `is_strong_v`. Dzięki temu Osoba B może od razu testować umiejętności na uproszczonej klasie `strong`, podczas gdy Osoba A pisze docelowe metaprogramowanie.



**Osoba A: Rdzeń `strong`, metaprogramowanie i rozwiązywanie zależności**
Ta część jest silnie nastawiona na operacje na listach typów w czasie kompilacji (type traits, rekurencja szablonowa).

* **Walidacja typu wartości `T`:** Napisanie konceptu/warunków sprawdzających, czy `T` jest typem arytmetycznym, klasowym lub wskaźnikiem oraz czy nie jest referencją, funkcją, `void`, tablicą C, klasą abstrakcyjną ani nie ma kwalifikatorów `const`/`volatile`.


* **Spłaszczanie `skills<...>`:** Implementacja szablonu `skills` oraz mechanizmu zamieniającego zagnieżdżone pakiety `skills` w płaską „jawną listę umiejętności”.


* **Walidacja jawnej listy umiejętności:** Sprawdzenie duplikatów umiejętności oraz konfliktów rodzin (`family`) na jawnej liście.


* **Algorytm uzupełniania zależności (BFS na typach):** Rozwijanie aliasów `needs` w kolejności wszerz, z pomijaniem umiejętności już obecnych lub mających reprezentanta tej samej rodziny na liście, wraz z obsługą cykli.


* **Walidacja pełnej listy umiejętności:** Sprawdzenie obecności szablonu `mixin` oraz warunku `accepts<T>` (z obsługą sytuacji, gdy `accepts`, `family` lub `needs` nie są zdefiniowane w danej umiejętności).


* **Szablon klasy `strong`:** Dziedziczenie po `Skill::mixin<strong<...>>`, pilnowanie rozmiaru `sizeof(strong) == sizeof(T)` (Empty Base Optimization), konstruktor domyślny, konstruktor z *perfect forwarding* (z blokadą konstruowania z innego silnego typu) oraz trywialna kopiowalność.


* **Dostęp i cechy typów:** Implementacja `skill_access::get` (z zachowaniem kategorii wartości lvalue/rvalue i `const`) oraz szablonów `is_strong_v`, `underlying_t` i `has_family_v`.



**Osoba B: Rodziny, umiejętności wbudowane (Mixins) i testy**
Ta część skupia się na wzorcu CRTP, *hidden friends*, ograniczeniach `requires`, literałach napisowych w szablonach i specjalizacji biblioteki standardowej.

* **Przestrzeń nazw `ops`:** Zdefiniowanie znaczników rodzin (`access`, `addition`, `scaling`, `equality`, `ordering`, `printing`, `hashing`).


* **Podstawowe umiejętności i arytmetyka:**
* `readable` (metoda `value()`).


* `addable_with<auto Op>` i alias `addable` (operatory `+`, `+=`, warunek `accepts<T>`).


* `scalable<typename Scalar>` (operatory `*`, `/`, `*=`, `/=`, warunek `accepts<T>`).


* `divisible_by<typename Other, typename Result, auto Op>` (obsługa dzielenia przez silny i zwykły typ).


* `convertible_to<typename Target, auto Conv>` (jawny operator konwersji do typu docelowego).




* **Porównywanie i zależność dynamiczna:**
* `equality_with<auto Eq>` i `equality`.


* `ordered_with<auto Cmp>` i `ordered` – w tym wygenerowanie w `needs` odpowiedniego `equality_with<E>` z lambdą sprawdzającą `Cmp(x, y) == 0`.




* **Wypisywanie i haszowanie:**
* `printable<"sufiks">` – implementacja pomocniczego typu strukturalnego przechowującego napis (aby obsłużyć literał napisowy jako argument szablonu NTTP w C++20/23) oraz operatora `<<`.


* `hashable_with<auto H>`, `hashable` oraz warunkowa specjalizacja `std::hash<S>` (aktywna tylko wtedy, gdy `S` ma umiejętność z rodziny `ops::hashing` lub odpowiednie wymagania).





**Wspólna finalizacja i przegląd kodu**

1. **Integracja:** Połączenie obu części w jednym pliku `strong.h`.


2. **Testy negatywne (błędy kompilacji):** Wspólne napisanie skryptu testującego przypadki, które *nie powinny* się kompilować (np. `std::hash` bez `hashable`, `<` bez `ordered`, konflikt rodzin na jawnej liście, błędne `T`, `skill_access::get` dla zwykłego `int`).


3. **Code review pod kątem kryteriów jakości:** Sprawdzenie `constexpr` (szczególnie z `std::string`), czytelności komunikatów w `static_assert`, braku duplikacji kodu w `accepts` oraz upewnienie się, że wszędzie użyto `requires` zamiast SFINAE i `using` zamiast `typedef`.



Czy macie już preferencje, kto z Was woli pisać metaprogramowanie na listach typów, a kto operatory i wzorzec CRTP?
