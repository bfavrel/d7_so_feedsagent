<?php

function so_feedsagent_date_definition($name, $options = array()) {
    
    return array(
        'translatable' => '1',
        'entity_types' => array('node'), // seul les content types doivent accéder à ce champ
        'settings' => array(
            'granularity' => array(
                'month' => 'month',
                'day' => 'day',
                'year' => 'year',
                'hour' => 0,
                'minute' => 0,
                'second' => 0,
            ),
            'tz_handling' => 'none',
            'timezone_db' => '',
            'cache_enabled' => 0,
            'cache_count' => '4',
            'repeat' => '0',
            'todate' => 'required',
        ),
        'storage' => array(
            'type' => 'field_sql_storage',
            'settings' => array(),
            'module' => 'field_sql_storage',
            'active' => '1',
            'details' => array(
                'sql' => array(
                    'FIELD_LOAD_CURRENT' => array(
                        'field_data_' . $name => array(
                            'value' => $name . '_value',
                            'value2' => $name . '_value2',
                        ),
                    ),
                    'FIELD_LOAD_REVISION' => array(
                        'field_revision_' . $name => array(
                            'value' => $name . '_value',
                            'value2' => $name . '_value2',
                        ),
                    ),
                ),
            ),
        ),
        'foreign keys' => array(),
        'indexes' => array(),
        'field_name' => $name,
        'type' => 'datetime',
        'module' => 'date',
        'active' => '1',
        'locked' => '0',
        'cardinality' => ($options['multiple'] == true ? FIELD_CARDINALITY_UNLIMITED : 1), // multiple = -1
        'deleted' => '0',
        'columns' => array(
            'value' => array(
                'type' => 'datetime',
                'mysql_type' => 'datetime',
                'pgsql_type' => 'timestamp without time zone',
                'sqlite_type' => 'varchar',
                'sqlsrv_type' => 'smalldatetime',
                'not null' => FALSE,
                'sortable' => TRUE,
                'views' => TRUE,
            ),
            'value2' => array(
                'type' => 'datetime',
                'mysql_type' => 'datetime',
                'pgsql_type' => 'timestamp without time zone',
                'sqlite_type' => 'varchar',
                'sqlsrv_type' => 'smalldatetime',
                'not null' => FALSE,
                'sortable' => TRUE,
                'views' => FALSE,
            ),
        ),
        'bundles' => array(
            'node' => array(), // on propose le champ dans tous les content types
        ),
    );    
}