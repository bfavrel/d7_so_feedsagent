<?php

/**
 * The field filter callback signature.
 *
 * IMPORTANT : if a feed field has no matching filter it will be automatically splited on separator set in config.
 * So it's not useful to define a such filter.
 * On the other hands, if a filter is defined it's its own responsability to split the values (since we have some "exotic" formats)
 *
 * @param array &$filtered_values : non-associative array of final values.
 *
 * @param array $tourinsoft_data : the Tourinsoft's raw values (indexed by feed field name) as they appear out of the XML parser.
 *                                 Useful to get also non unionized fields' values.
 *                                 The present field's value is get with "$tourinsoft_data['TOURINSOFT_FIELD_NAME']"
 *
 * @param array &$fields : the virtual type fields' array.
 *                         Useful if a filter needs to alter other fields.
 *                         '$filtered_values' (above) can be accessed by $fields[TOURINSOFT FIELD]['filtered_values'] entries.
 *
 * @param object $node : the traditional node object. Here, you can populate foreign fields.
 *
 * @param array $connector_params : connector's configuration.
 *                                  "$connector_params['data_import']['separator']" should be useful.
 *                                  "$connector_params['webservice']['media_url']" should be too.
 *
 * @param array $virtual_type_params : idModule, OBJETTOUR, ...
 */
function sfa_TOURINSOFT_FIELD_NAME_field_filter(&$filtered_values, $tourinsoft_data, &$fields, $node, $connector_params, $virtual_type_params) {}

/**
 * The field type filter callback signature.
 *
 * Same parameters as above, except for :
 * @param string $tourinsoft_field_name : the raw field name. Use "$tourinsoft_data[$tourinsoft_field_name]" to get the current field value.
 *
 * @see so_feedsagent.module:1115:so_feedsagent_get_available_fields_types() for type names.
 */
function sfa_SFA_FIELD_TYPE_type_filter(&$filtered_values, $tourinsoft_field_name, $tourinsoft_data, &$fields, $node, $connector_params, $virtual_type_params) {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+ FIELDS                                                                                                                             +
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*
 * ACHTUNG ! the filter seems a little messy :
 * "Hôtel-restaurant du Château" (HOTAQU024FS0000A) :
 *
 * Chambre double||||59|87|Chambre single||||46|59|Demi-pension|par personne|||32|32|Chambre familiale| ou suite|||82|113|Petit-déjeuner||||8|8
 * ->
 * "Chambre double :  € (2013)"
 * "Chambre double : from 59 € to 87 € (2014)
 * "..."
 */
function sfa_tarif_field_filter(&$filtered_values, $tourinsoft_data, &$fields, $node, $connector_params, $virtual_type_params) {

    $line_size = 6;

	if (empty($tourinsoft_data['tarif'])) {
		return;
    }

	// Transformation de la chaine de caractère en tableau (termes séparés par "|" )
	$values = explode($connector_params['data_import']['separator'], $tourinsoft_data['tarif']);

	if ($values[0] == "") {
		return;
    }

	// Préparation pour la traduction
	$from = t('from', array(), array('langcode' => $node->language));
	$to = t('to', array(), array('langcode' => $node->language));

	$line_nb = count($values);
	$tarif_min = floatval(preg_replace('/,/', '.', $values[2]));
	$tarif_max = floatval(preg_replace('/,/', '.', $values[3]));

	for ($i = 0; $i < $line_nb; $i += $line_size) {

		$line = "";

		if ($values[$i+2] != '') {
			if ($tarif_min > floatval(preg_replace('/,/', '.', $values[$i+2])))
				$tarif_min = floatval(preg_replace('/,/', '.', $values[$i+2]));
		} else {
			if ($tarif_min > floatval(preg_replace('/,/', '.', $values[$i+3])))
				$tarif_min = floatval(preg_replace('/,/', '.', $values[$i+3]));
		}

		if ($values[$i+3] != '') {

			if ($tarif_max < floatval(preg_replace('/,/', '.', $values[$i+3])))
				$tarif_max = floatval(preg_replace('/,/', '.', $values[$i+3]));

			if ($values[$i+2] == $values[$i+3] || $values[$i+2] == '') {

				if ($values[$i+1] != '') {
					// Intitulé (complément tarif) : X €
					$line = $values[$i] . ' (' . $values[$i+1] . ') : ' . $values[$i+3] . ' € ';
				}
				else {
					// Intitulé : X €
					$line = $values[$i] . ' : ' . $values[$i+3] . ' € ';
				}
			}
			else {
				if ($values[$i+1] != '') {
					// Intitulé (complément tarif) : de X € à Y €
					$line = $values[$i] . ' (' . $values[$i+1] . ') : ' . $from . ' ' . $values[$i+2] . ' € ' . $to . ' ' . $values[$i+3] . ' €';
				}
				else {
					// Intitulé : de X € à Y €
					$line = $values[$i] . ' : ' . $from . ' ' . $values[$i+2] . ' € ' . $to . ' ' . $values[$i+3] . ' €';
				}
			}
		}
		else {
			if ($values[$i+1] != '') {
				// Intitulé (complément tarif) : X €
				$line = $values[$i] . ' (' . $values[$i+1] . ') : ' . $values[$i+2] . ' €';
			}
			else {
				// Intitulé : X €
				$line = $values[$i] . ' : ' . $values[$i+2] . ' €';
			}
		}

		if ($values[$i+4] != '') {
			// Intitulé (complément tarif) : de X € à Y € (ANNEE N)
			$line .= ' (' . date('Y') . ')';
			$filtered_values[] = $line; // on stocke simplement la valeur comme entrée du tableau
			if ($values[$i+5] != '') {
				if ($values[$i+1] != '') {
					// Intitulé (complément tarif) : de X € à Y € (ANNEE N+1)
					$line = $values[$i] . ' (' . $values[$i+1] . ') : ' . $from . ' ' . $values[$i+4] . ' € ' . $to . ' ' . $values[$i+5] . ' € (' . (date('Y')+1) . ')';
				}
				else {
					// Intitulé : de X € à Y € (ANNEE N+1)
					$line = $values[$i] . ' : ' . $from . ' ' . $values[$i+4] . ' € ' . $to . ' ' . $values[$i+5] . ' € (' . (date('Y')+1) . ')';
				}
			}
			else {
				if ($values[$i+1] != '') {
					// Intitulé (complément tarif) : X €
					$line = $values[$i] . ' (' . $values[$i+1] . ') : ' . $values[$i+4] . ' € (' . (date('Y')+1) . ')';
				}
				else {
					// Intitulé : X €
					$line = $values[$i] . ' : ' . $values[$i+4] . ' € (' . (date('Y')+1) . ')';
				}
			}
		}
		$filtered_values[] = $line; // on stocke simplement la valeur comme entrée du tableau
	}

    // Ici, nous avons des champs non-gérés par SFA (créés manuellement). Leurs valeurs doivent être inscrites
    // directement dans l'objet $node, avec le format habituel :

	if ($tarif_min > 0) {
		$node->field_tarif_min[$node->language][0] = array('value' => $tarif_min);
	}

	if ($tarif_max > 0) {
		$node->field_tarif_max[$node->language][0] = array('value' => $tarif_max);
	}
}

/*
 * NOTA : for field of 'image' type, each final value can be a string (filename) or
 * associative array : ['file'] => filename, ['title'] => image title, ['description'] => image description
 */
function sfa_photos_field_filter(&$filtered_values, $tourinsoft_data, &$fields, $node, $connector_params, $virtual_type_params) {

	$tourinsoft_data['photo'] = $tourinsoft_data['photos'];

	sfa_photo_field_filter($filtered_values, $tourinsoft_data, $fields, $node, $connector_params, $virtual_type_params);
}

function sfa_photo_field_filter(&$filtered_values, $tourinsoft_data, &$fields, $node, $connector_params, $virtual_type_params) {

	if (empty($tourinsoft_data['photo'])) {
		return;
	}

	// Transformation de la chaine de caractère en tableau (termes séparés par "|" )
	$values = explode($connector_params['data_import']['separator'], $tourinsoft_data['photo']);

	if ($values[1] == "") {
		return;
	}

  $occ_size = 3;
	$occ_nb = count($values);

	for ($i = 0; $i < $occ_nb; $i += $occ_size) {
		$file = trim($values[$i + 1]);

        if ($file == "") {continue;}

        $filtered_values[] = array(
            'file' => $connector_params['webservice']['media_url'] . '/' . $file,
            'title' => $values[$i],
        		'alt' => $values[$i],
            'description' => $values[$i+2],
        );
    }
}

function sfa_video_field_filter(&$filtered_values, $tourinsoft_data, &$fields, $node, $connector_params, $virtual_type_params) {

    if (empty($tourinsoft_data['video'])) {
		return;
	}

    // Transformation de la chaine de caractère en tableau (termes séparés par "|" )
	$values = explode($connector_params['data_import']['separator'], $tourinsoft_data['video']);

    foreach($values as $url) {
        $clean_url = trim($url);

        if(empty($clean_url)) {continue;}

        $field_language = $fields['video']['translatable'] == 1 ? $node->language : 'und';

        $filtered_values[] = array(
            'video_url' => $clean_url,
        );
    }
}

function sfa_tracekmz_field_filter(&$filtered_values, $tourinsoft_data, &$fields, $node, $connector_params, $virtual_type_params) {

	if (empty($tourinsoft_data['tracekmz'])) {
		return;
	}

	// Transformation de la chaine de caractère en tableau (termes séparés par "|" )
	$values = explode($connector_params['data_import']['separator'], $tourinsoft_data['tracekmz']);

	if ($values[0] == "") {
		return;
	}

	$file = trim($values[0]);

	if ($file == "") {return;}

	$filtered_values[] = array(
			'file' => $connector_params['webservice']['media_url'] . '/' . $file,
			'title' => $node->title
	);
}

function sfa_tracegpx_field_filter(&$filtered_values, $tourinsoft_data, &$fields, $node, $connector_params, $virtual_type_params) {

	if (empty($tourinsoft_data['tracegpx'])) {
		return;
	}

	// Transformation de la chaine de caractère en tableau (termes séparés par "|" )
	$values = explode($connector_params['data_import']['separator'], $tourinsoft_data['tracegpx']);

	if ($values[0] == "") {
		return;
	}

	$file = trim($values[0]);

	if ($file == "") {return;}

	$filtered_values[] = array(
			'file' => $connector_params['webservice']['media_url'] . '/' . $file,
			'title' => $node->title
	);
}

function sfa_pdf_field_filter(&$filtered_values, $tourinsoft_data, &$fields, $node, $connector_params, $virtual_type_params) {

	if (empty($tourinsoft_data['pdf'])) {
		return;
	}

	// Transformation de la chaine de caractère en tableau (termes séparés par "|" )
	$values = explode($connector_params['data_import']['separator'], $tourinsoft_data['pdf']);

	if ($values[0] == "") {
		return;
	}

	$file = trim($values[0]);

	if ($file == "") {return;}

	$filtered_values[] = array(
			'file' => $connector_params['webservice']['media_url'] . '/' . $file,
			'title' => $node->title
	);
}


/**
 * Les valeurs données par SIRTAqui sont :
 * Date de début année N | date de fin année N | heure d'ouverture 1 | heure fermeture 1 | heure d'ouverture 2 | heure fermeture 2 | jour fermeture |
 * Date de début année N+1 | date de fin année N+1 | heure d'ouverture 1 (N+1) | heure fermeture 1 (N+1) | heure d'ouverture 2 (N+1) | heure fermeture 2 (N+1)
 */
function sfa_ouverture_field_filter(&$filtered_values, $tourinsoft_data, &$fields, $node, $connector_params, $virtual_type_params) {

	if (empty($tourinsoft_data['ouverture'])) {
		return;
	}

	// Transformation de la chaine de caractère en tableau (termes séparés par "|" )
	$ouverture = explode($connector_params['data_import']['separator'], $tourinsoft_data['ouverture']);

	array_walk($ouverture, function(&$val){
		$val = trim($val);
	});

	if ($ouverture[0] == "") {
		return;
	}

	$dd = 0;
	$df = 1;
	$ho1 = 2;
	$hf1 = 3;
	$ho2 = 4;
	$hf2 = 5;
	$jf = 6;
	$ddn = 7;
	$dfn = 8;
	$ho1n = 9;
	$hf1n = 10;
	$ho2n = 11;
	$hf2n = 12;

	$line_size = 13;

	$line_nb = count($ouverture);

	for ($i = 0; $i < $line_nb; $i += $line_size) {

		// Le DATE | Du DATE au DATE
		if ($ouverture[$i + $dd] != '' && $ouverture[$i + $df] != '' && $ouverture[$i + $ho1] == '' && $ouverture[$i + $hf1] == '' && $ouverture[$i + $ho2] == '' && $ouverture[$i + $hf2] == '') {
			if ($ouverture[$i + $dd] == $ouverture[$i + $df]) {
				$filtered_values[] = array('value' => t('The @begindate', array('@begindate' => $ouverture[$i + $dd]), array('langcode' => $node->language)));
			}
			else {
				$filtered_values[] = array('value' => t('From @begindate to @enddate', array('@begindate' => $ouverture[$i + $dd], '@enddate' => $ouverture[$i + $df]), array('langcode' => $node->language)));
			}
		} // Le DATE, à HEURE || Du DATE au DATE, à HEURE
		elseif ($ouverture[$i + $dd] != '' && $ouverture[$i + $df] != '' && $ouverture[$i + $ho1] != '' && $ouverture[$i + $hf1] == '' && $ouverture[$i + $ho2] == '' && $ouverture[$i + $hf2] == '') {
			if ($ouverture[$i + $dd] == $ouverture[$i + $df]) {
				$filtered_values[] = array('value' => t('The @begindate, at @openhour', array('@begindate' => $ouverture[$i + $dd], '@openhour' => $ouverture[$i + $ho1]), array('langcode' => $node->language)));
			}
			else {
				$filtered_values[] = array('value' => t('From @begindate to @enddate, at @openhour', array('@begindate' => $ouverture[$i + $dd], '@enddate' => $ouverture[$i + $df], '@openhour' => $ouverture[$i + $ho1]), array('langcode' => $node->language)));
			}
		} // Le DATE, de HEURE à HEURE || Le DATE, à HEURE || Du DATE au DATE, à HEURE || Du DATE au DATE, de HEURE à HEURE
		elseif ($ouverture[$i + $dd] != '' && $ouverture[$i + $df] != '' && $ouverture[$i + $ho1] != '' && $ouverture[$i + $hf1] != '' && $ouverture[$i + $ho2] == '' && $ouverture[$i + $hf2] == '') {
			if ($ouverture[$i + $dd] == $ouverture[$i + $df] && $ouverture[$i + $ho1] != $ouverture[$i + $hf1]) {
				$filtered_values[] = array('value' => t('The @begindate, from @openhour to @closurehour', array('@begindate' => $ouverture[$i + $dd], '@openhour' => $ouverture[$i + $ho1], '@closurehour' => $ouverture[$i + $hf1]), array('langcode' => $node->language)));
			}
			elseif ($ouverture[$i + $dd] == $ouverture[$i + $df] && $ouverture[$i + $ho1] == $ouverture[$i + $hf1]) {
				$filtered_values[] = array('value' => t('The @begindate, at @openhour', array('@begindate' => $ouverture[$i + $dd], '@openhour' => $ouverture[$i + $ho1]), array('langcode' => $node->language)));
			}
			elseif ($ouverture[$i + $dd] != $ouverture[$i + $df] && $ouverture[$i + $ho1] == $ouverture[$i + $hf1]) {
				$filtered_values[] = array('value' => t('From @begindate to @enddate, at @openhour', array('@begindate' => $ouverture[$i + $dd], '@enddate' => $ouverture[$i + $df], '@openhour' => $ouverture[$i + $ho1]), array('langcode' => $node->language)));
			}
			else {
				$filtered_values[] = array('value' => t('From @begindate to @enddate, from @openhour to @closurehour', array('@begindate' => $ouverture[$i + $dd], '@enddate' => $ouverture[$i + $df], '@openhour' => $ouverture[$i + $ho1], '@closurehour' => $ouverture[$i + $hf1]), array('langcode' => $node->language)));
			}
		} // Le DATE, du HEURE à HEURE et du HEURE à HEURE || Du DATE au DATE, de HEURE à HEURE et de HEURE à HEURE
		elseif ($ouverture[$i + $dd] != '' && $ouverture[$i + $df] != '' && $ouverture[$i + $ho1] != '' && $ouverture[$i + $hf1] != '' && $ouverture[$i + $ho2] != '' && $ouverture[$i + $hf2] != '') {
			if ($ouverture[$i + $dd] == $ouverture[$i + $df]) {
				$filtered_values[] = array('value' => t('The @begindate, from @openhour to @closurehour and from @openhour2 to @closurehour2', array('@begindate' => $ouverture[$i + $dd], '@openhour' => $ouverture[$i + $ho1], '@closurehour' => $ouverture[$i + $hf1], '@openhour2' => $ouverture[$i + $ho2], '@closurehour2' => $ouverture[$i + $hf2]), array('langcode' => $node->language)));
			}
			else {
				$filtered_values[] = array('value' => t('From @begindate to @enddate, from @openhour to @closurehour and from @openhour2 to @closurehour2', array('@begindate' => $ouverture[$i + $dd], '@enddate' => $ouverture[$i + $df], '@openhour' => $ouverture[$i + $ho1], '@closurehour' => $ouverture[$i + $hf1], '@openhour2' => $ouverture[$i + $ho2], '@closurehour2' => $ouverture[$i + $hf2]), array('langcode' => $node->language)));
			}
		}
		if ($ouverture[$i + $jf] != '') {
			$filtered_values[] = array('value' => t('Closed on @closedday', array('@closedday' => $ouverture[$i + $jf])));
		}
		if (isset($ouverture[$i + $ddn]) && $ouverture[$i + $ddn] != '') {
			// Le DATE | Du DATE au DATE
			if ($ouverture[$i + $ddn] != '' && $ouverture[$i + $dfn] != '' && $ouverture[$i + $ho1n] == '' && $ouverture[$i + $hf1n] == '' && $ouverture[$i + $ho2n] == '' && $ouverture[$i + $hf2n] == '') {
				if ($ouverture[$i + $ddn] == $ouverture[$i + $dfn]) {
					$filtered_values[] = array('value' => t('The @begindate', array('@begindate' => $ouverture[$i + $ddn]), array('langcode' => $node->language)));
				}
				else {
					$filtered_values[] = array('value' => t('From @begindate to @enddate', array('@begindate' => $ouverture[$i + $ddn], '@enddate' => $ouverture[$i + $dfn]), array('langcode' => $node->language)));
				}
			} // Le DATE, à HEURE || Du DATE au DATE, à HEURE
			elseif ($ouverture[$i + $ddn] != '' && $ouverture[$i + $dfn] != '' && $ouverture[$i + $ho1n] != '' && $ouverture[$i + $hf1n] == '' && $ouverture[$i + $ho2n] == '' && $ouverture[$i + $hf2n] == '') {
				if ($ouverture[$i + $ddn] == $ouverture[$i + $dfn]) {
					$filtered_values[] = array('value' => t('The @begindate, at @openhour', array('@begindate' => $ouverture[$i + $ddn], '@openhour' => $ouverture[$i + $ho1n]), array('langcode' => $node->language)));
				}
				else {
					$filtered_values[] = array('value' => t('From @begindate to @enddate, at @openhour', array('@begindate' => $ouverture[$i + $ddn], '@enddate' => $ouverture[$i + $dfn], '@openhour' => $ouverture[$i + $ho1n]), array('langcode' => $node->language)));
				}
			} // Le DATE, de HEURE à HEURE || Le DATE, à HEURE || Du DATE au DATE, à HEURE || Du DATE au DATE, de HEURE à HEURE
			elseif ($ouverture[$i + $ddn] != '' && $ouverture[$i + $dfn] != '' && $ouverture[$i + $ho1n] != '' && $ouverture[$i + $hf1n] != '' && $ouverture[$i + $ho2n] == '' && $ouverture[$i + $hf2n] == '') {
				if ($ouverture[$i + $ddn] == $ouverture[$i + $dfn] && $ouverture[$i + $ho1n] != $ouverture[$i + $hf1n]) {
					$filtered_values[] = array('value' => t('The @begindate, from @openhour to @closurehour', array('@begindate' => $ouverture[$i + $ddn], '@openhour' => $ouverture[$i + $ho1n], '@closurehour' => $ouverture[$i + $hf1n]), array('langcode' => $node->language)));
				}
				elseif ($ouverture[$i + $ddn] == $ouverture[$i + $dfn] && $ouverture[$i + $ho1n] == $ouverture[$i + $hf1n]) {
					$filtered_values[] = array('value' => t('The @begindate, at @openhour', array('@begindate' => $ouverture[$i + $ddn], '@openhour' => $ouverture[$i + $ho1n]), array('langcode' => $node->language)));
				}
				elseif ($ouverture[$i + $ddn] != $ouverture[$i + $dfn] && $ouverture[$i + $ho1n] == $ouverture[$i + $hf1n]) {
					$filtered_values[] = array('value' => t('From @begindate to @enddate, at @openhour', array('@begindate' => $ouverture[$i + $ddn], '@enddate' => $ouverture[$i + $dfn], '@openhour' => $ouverture[$i + $ho1n]), array('langcode' => $node->language)));
				}
				else {
					$filtered_values[] = array('value' => t('From @begindate to @enddate, from @openhour to @closurehour', array('@begindate' => $ouverture[$i + $ddn], '@enddate' => $ouverture[$i + $dfn], '@openhour' => $ouverture[$i + $ho1n], '@closurehour' => $ouverture[$i + $hf1n]), array('langcode' => $node->language)));
				}
			} // Le DATE, du HEURE à HEURE et du HEURE à HEURE || Du DATE au DATE, de HEURE à HEURE et de HEURE à HEURE
			elseif ($ouverture[$i + $ddn] != '' && $ouverture[$i + $dfn] != '' && $ouverture[$i + $ho1n] != '' && $ouverture[$i + $hf1n] != '' && $ouverture[$i + $ho2n] != '' && $ouverture[$i + $hf2n] != '') {
				if ($ouverture[$i + $ddn] == $ouverture[$i + $dfn]) {
					$filtered_values[] = array('value' => t('The @begindate, from @openhour to @closurehour and from @openhour2 to @closurehour2', array('@begindate' => $ouverture[$i + $ddn], '@openhour' => $ouverture[$i + $ho1n], '@closurehour' => $ouverture[$i + $hf1n], '@openhour2' => $ouverture[$i + $ho2n], '@closurehour2' => $ouverture[$i + $hf2n]), array('langcode' => $node->language)));
				}
				else {
					$filtered_values[] = array('value' => t('From @begindate to @enddate, from @openhour to @closurehour and from @openhour2 to @closurehour2', array('@begindate' => $ouverture[$i + $ddn], '@enddate' => $ouverture[$i + $dfn], '@openhour' => $ouverture[$i + $ho1n], '@closurehour' => $ouverture[$i + $hf1n], '@openhour2' => $ouverture[$i + $ho2n], '@closurehour2' => $ouverture[$i + $hf2n]), array('langcode' => $node->language)));
				}
			}
		}

	}

}

/**
 * Les valeurs données par SIRTAqui sont :
 * Date de début année N | date de fin année N | Date de début année N+1 | date de fin année N+1
 */
function sfa_periodeouverture_field_filter(&$filtered_values, $tourinsoft_data, &$fields, $node, $connector_params, $virtual_type_params) {

	if (empty($tourinsoft_data['periodeouverture'])) {
		return;
	}

	// Transformation de la chaine de caractère en tableau (termes séparés par "|" )
	$values = explode($connector_params['data_import']['separator'], $tourinsoft_data['periodeouverture']);

	if ($values[0] == "") {
		return;
	}

	$occ_size = 2;
	$occ_nb = count($values);

	for ($i = 0; $i < $occ_nb; $i += $occ_size) {

		if ($values[$i] != '' && $values[$i+1] != '') {
			$date = DateTime::createFromFormat('d/m/Y', $values[$i]);
			$date2 = DateTime::createFromFormat('d/m/Y', $values[$i+1]);
			$filtered_values[] = array(
					$date->format('Y-m-d H:i:s'),
					$date2->format('Y-m-d H:i:s')
			);
		}
	}
}

function sfa_labelth_field_filter(&$filtered_values, $tourinsoft_data, &$fields, $node, $connector_params, $virtual_type_params) {

	if (empty($tourinsoft_data['labelth'])) {
		return;
	}

	// Transformation de la chaine de caractère en tableau (termes séparés par "-" )
	$values = explode(' - ', $tourinsoft_data['labelth']);

	if ($values[0] == "") {
		return;
	}

	foreach ($values as $value) {
		$filtered_values[] = _tourinsoft_img2text($value);
	}

}

function sfa_marques_field_filter(&$filtered_values, $tourinsoft_data, &$fields, $node, $connector_params, $virtual_type_params) {

	if (empty($tourinsoft_data['marques'])) {
		return;
	}

	// Transformation de la chaine de caractère en tableau (termes séparés par "|" )
	$values = explode($connector_params['data_import']['separator'], $tourinsoft_data['marques']);

	if ($values[0] == "") {
		return;
	}

	foreach ($values as $value) {
		$filtered_values[] = _tourinsoft_img2text($value);
	}
}

function sfa_categorie_field_filter(&$filtered_values, $tourinsoft_data, &$fields, $node, $connector_params, $virtual_type_params) {

	if (empty($tourinsoft_data['categorie'])) {
		return;
	}

	// Transformation de la chaine de caractère en tableau (termes séparés par "|" )
	$values = explode($connector_params['data_import']['separator'], $tourinsoft_data['categorie']);

	if ($values[0] == "") {
		return;
	}

	foreach ($values as $value) {
		$filtered_values[] = _tourinsoft_img2text($value);
	}
}

function sfa_chaines_field_filter(&$filtered_values, $tourinsoft_data, &$fields, $node, $connector_params, $virtual_type_params) {

	if (empty($tourinsoft_data['chaines'])) {
		return;
	}

	// Transformation de la chaine de caractère en tableau (termes séparés par "|" )
	$values = explode($connector_params['data_import']['separator'], $tourinsoft_data['chaines']);

	if ($values[0] == "") {
		return;
	}

	foreach ($values as $value) {
		$filtered_values[] = _tourinsoft_img2text($value);
	}
}

function sfa_logis_field_filter(&$filtered_values, $tourinsoft_data, &$fields, $node, $connector_params, $virtual_type_params) {

	if (empty($tourinsoft_data['logis'])) {
		return;
	}

	// Transformation de la chaine de caractère en tableau (termes séparés par "|" )
	$values = explode($connector_params['data_import']['separator'], $tourinsoft_data['logis']);

	if ($values[0] == "") {
		return;
	}

	foreach ($values as $value) {
		$filtered_values[] = _tourinsoft_img2text($value);
	}
}

function sfa_classlabel_field_filter(&$filtered_values, $tourinsoft_data, &$fields, $node, $connector_params, $virtual_type_params) {

	if (empty($tourinsoft_data['classlabel'])) {
		return;
	}

	// Transformation de la chaine de caractère en tableau (termes séparés par "|" )
	$values = explode($connector_params['data_import']['separator'], $tourinsoft_data['classlabel']);

	if ($values[0] == "") {
		return;
	}

	foreach ($values as $value) {
		$filtered_values[] = _tourinsoft_img2text($value);
	}
}

function sfa_duree_field_filter(&$filtered_values, $tourinsoft_data, &$fields, $node, $connector_params, $virtual_type_params) {

	if (empty($tourinsoft_data['logis'])) {
		return;
	}

	// Transformation de la chaine de caractère en tableau (termes séparés par "|" )
	$values = explode($connector_params['data_import']['separator'], $tourinsoft_data['logis']);

	if ($values[0] == "") {
		return;
	}

	$occ_size = 2;
	$occ_nb = count($values);

	for ($i = 0; $i < $occ_nb; $i += $occ_size) {

		if ($values[$i] != '' && $values[$i+1] != '') {
			$filtered_values[] = array(
					'value' => t('@locomotion @duree', array('@locomotion' => $values[$i], '@duree' => $values[$i+1]), array('langcode' => $node->language))
			);
		}
	}
}

function sfa_activitesportive_field_filter(&$filtered_values, $tourinsoft_data, &$fields, $node, $connector_params, $virtual_type_params) {

	if (empty($tourinsoft_data['activitesportive'])) {
		return;
	}
	if ($node->type == 'srt_activitesetequipementsdelois') {
		$filtered_values = explode($connector_params['data_import']['separator'], $tourinsoft_data['activitesportive']);
	}

	// Transformation de la chaine de caractère en tableau (termes séparés par "|" )
	$values = explode($connector_params['data_import']['separator'], $tourinsoft_data['activitesportive']);

	if ($values[0] == "") {
		return;
	}

	$occ_size = 4;
	$occ_nb = count($values);

	for ($i = 0; $i < $occ_nb; $i += $occ_size) {

		if ($values[$i+1] != '' && $values[$i+2] != '') {
			$filtered_values[] = t('@activite (@detail)', array('@activite' => $values[$i+1], '@detail' => $values[$i+2]), array('langcode' => $node->language));
		} elseif ($values[$i+1] != '') {
			$filtered_values[] = $values[$i+1];
		}
	}
}

/**
 * Get text value for imgs
 */
function _tourinsoft_img2text($value) {

	if(empty($value)) {return $value;}

	$dom = new DOMDocument();
	$dom->loadHTML($value);

	if ($dom->getElementsByTagName('img')->length > 0) {

		return utf8_decode($dom->getElementsByTagName('img')->item(0)->getAttribute('title'));
	}

	return $value;
}

/*
 * A better presentation for Tourinsoft system's fields.
 *
 * -> 'Y-m-d H:i:s'
 */
function _tourinsoft_date2text($date) {

    if(empty($date)) {return array();}

    $date_object = DateTime::createFromFormat('Y-m-d\TH:i:s.u0P', $date);

    if($date_object == false) {
        $date_object = DateTime::createFromFormat('Y-m-d\TH:i:sP', $date);
    }

    if($date_object == false) {
        $date_object = DateTime::createFromFormat('Y-m-d\TH:i:s.uP', $date);
    }

    return array($date_object->format('d/m/Y H:i:s'));
}
function sfa_DATECREA_field_filter(&$filtered_values, $tourinsoft_data, &$fields, $node, $connector_params, $virtual_type_params) {
    $filtered_values = _tourinsoft_date2text($tourinsoft_data['DATECREA']);
}
function sfa_DATEMAJ_field_filter(&$filtered_values, $tourinsoft_data, &$fields, $node, $connector_params, $virtual_type_params) {
    $filtered_values = _tourinsoft_date2text($tourinsoft_data['DATEMAJ']);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+ FIELD TYPES                                                                                                                        +
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*
 * Checkbox on/off example.
 *
 * Field API is very strict with types of values. It looks for an integer and nothing else for on/off checkboxes.
 * A "oui" value insn't converted as an integer, so PDO throws an exception, and content creation aborts.
 */
function sfa_onoff_type_filter(&$filtered_values, $tourinsoft_field_name, $tourinsoft_data, &$fields, $node, $connector_params, $virtual_type_params) {

    if($tourinsoft_data[$tourinsoft_field_name] == 'oui') {
        $filtered_values[] = 1;
    } else {
        $filtered_values[] = 0;
    }
}

/*
 * Textarea
 * Makes HTML line feed.
 */
function sfa_textarea_type_filter(&$filtered_values, $tourinsoft_field_name, $tourinsoft_data, &$fields, $node, $connector_params, $virtual_type_params) {
    
    $filtered_values[] = nl2br($tourinsoft_data[$tourinsoft_field_name]);
}