# Search Server
Поисковая система

## Описание
Поисковый сервер обеспечивает комплексный поиск документов по ключевым словам, учитывая стоп-слова и статус документа. Алгоритм поиска основан на статистике TF-IDF с поддержкой параллельного выполнения.

Также реализован класс Paginator, который помогает разбить результаты поиска на несколько страниц.

При добавлении документов может быть включено несколько дубликатов. Функция RemoveDuplicates осуществляет поиск и удаление дубликатов.

Поисковый сервер оптимизирован для высокой производительности и низкого использования памяти благодаря усовершенствованным алгоритмам, string_view и использованию семантики перемещения.

## Применяемые навыки
ООП, шаблоны, лямбда-функции, стандартные алгоритмы, параллельные вычисления, многопоточные операции, TDD.
