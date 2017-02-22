<?php

class SoFeedsAgentAC3Field_CONSOMMATIONENERGETIQUE implements SoFeedsAgentAC3Field_Interface
{
    public static function getFieldDefinition() {
        return array(
            'label' => "Consommation énergétique",
            'type' => 'select',
            'multiple' => false,
            'values' => array(
                'A' => "A",
                'B' => "B",
                'C' => "C",
                'D' => "D",
                'E' => "E",
                'F' => "F",
                'G' => "G"
            ),
        );
    }
}