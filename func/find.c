//
// Created by schumann on 4/27/21.
//

/*
 * ### Suchfunktion für LinkedList
 * Eine Binäre Suche für LinkedList
 * Anforderungen:
 *  - Benötigt wird der Start der Liste.
 *  - Die Liste muss sortiert sein.
 *  - Es wird die momentane Gesamtanzahl der Elemente der Liste benötigt.
 *
 *  Ablauf:
 *  Die Suche beginnt in der Mitte (GesamtanzahlElemente / 2, wenn ungerade nächst höhere Wert)
 *  Prüfen ob der Mittelwert klein oder größer ist als gesuchter Key.
 *   - kleiner, die Suche beginnt von Vorn und verwendet Mitte als neue GesamtanzahlElemente
 *   - größer, die Suche beginnt von Vorn und verwendet Mitte als Start und Gesamtanzahl als Ende.
 *      neue Mitte ist (Gesamtanzahl-Mitte)/2
 *
 */

int find(char* key, int Gesamt) {


};