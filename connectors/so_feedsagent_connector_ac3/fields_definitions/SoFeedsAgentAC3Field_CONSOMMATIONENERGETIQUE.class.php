<?php

class SoFeedsAgentAC3Field_SS_TYPE implements SoFeedsAgentAC3Field_Interface
{
    public static function getFieldDefinition() {
        return array(
            'label' => "Sous type de bien",
            'type' => 'select',
            'multiple' => false,
            'values' => self::getValuesArray(),
        );
    }

    private static function getValuesArray() {
        return array(
            "1" => "Appartement",
            "25" => "Bastide / Mas",
            "Bergerie" => "Bergerie",
            "11" => "Bureau / Local professionnel",
            "12" => "Cession de bail",
            "28" => "Chalet",
            "Chateau" => "Château",
            "22" => "Corps de ferme",
            "3" => "Demeure de Prestige",
            "7" => "Domaine agricole",
            "8" => "Domaine forestier",
            "Viticole" => "Domaine viticole",
            "13" => "Entrepôt / Local industriel",
            "15" => "Entreprise",
            "18" => "Etang",
            "4" => "Fonds de commerce",
            "Gtrange" => "Grange",
            "Ameau" => "Hameau",
            "Hotel_Part" => "Hotel particulier",
            "6" => "Immeuble",
            "23" => "Local commercial",
            "27" => "Local d'activité",
            "Loft" => "Loft",
            "16" => "Longère",
            "26" => "Lotissement",
            "2" => "Maison",
            "30" => "Maison de village",
            "21" => "Manoir",
            "31" => "Mas",
            "20" => "Moulin",
            "14" => "Murs",
            "5" => "Parking / box",
            "PN" => "Programme Neuf",
            "19" => "Riad",
            "10" => "Terrain",
            "24" => "Terrain de loisirs",
            "17" => "Villa",
            "33" => "Cave",
            "ExploitationAgricole" => "Exploitation Agricole",
        );
    }
}