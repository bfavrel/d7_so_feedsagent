<?php

class SoFeedsAgentAC3Field_MODE_CHAUFFAGE implements SoFeedsAgentAC3Field_Interface {

    public static function getFieldDefinition() {
        return array(
            'label' => "Mode de chauffage",
            'type' => 'select',
            'multiple' => false,
            'values' => self::getValuesArray(),
        );
    }

    private static function getValuesArray() {
        return array(
            'Aerothermie' => "Aérothermie",
            'Aucun' => "Aucun",
            'Autres' => "Autres",
            'Bois' => "Bois",
            'Bois_Electrique' => "Bois + Electrique",
            'Chauffagedeville' => "Chauffage de ville",
            'Climatisation_Reversible' => "Climatisation réversible",
            'Electrique' => "Electrique",
            'Fuel' => "Fuel",
            'Gaz' => "Gaz",
            'Turbine' => "Gaz (turbine)",
            'Gazdeville' => "Gaz de ville",
            'Geothermie' => "Géothermie",
            'PanneauxSolaires' => "Panneaux solaires",
        );
    }

}
