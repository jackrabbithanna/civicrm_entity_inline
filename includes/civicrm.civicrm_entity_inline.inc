<?php

class CivicrmInlineEntityFormController extends EntityInlineEntityFormController {
  /**
   * Overrides EntityInlineEntityFormController::entityForm().
   */
  public function entityForm($entity_form, &$form_state) {
    $entity_type = $form_state['entity_type'] = $entity_form['#entity_type'];
    $entity = $form_state['entity'] = $entity_form['#entity'];
    $entity->type = $entity_type;
    $op = $entity_form['#op'];

    field_attach_form($entity_type, $entity, $entity_form, $form_state);

    if ($op == 'edit') {
      $wrapper = entity_metadata_wrapper($entity_type, $entity);
    }
    else {
      $wrapper = entity_metadata_wrapper($entity_type);
    }

    foreach ($wrapper as $name => $child) {
      $info = $child->info();
      if (strpos($info['name'], 'field_') === 0) {
        continue;
      }
      else {
       if(isset($info['widget']['widget'])){
          $entity_form[$name] = array(
            '#type' => $info['widget']['widget'],
            '#title' => $info['label'],
            '#description' => !empty($info['description']) ? $info['description'] : '',
            '#default_value' => $op == 'edit' ? $child->value() : '',
          );
          if(isset($info['required']) && $info['required'] == TRUE) {
            $entity_form[$name]['#required'] = TRUE;
          }
          //set the options for select options
          if($entity_form[$name]['#type'] == 'select') {
            $entity_form[$name]['#options'] = $info['widget']['options'];
          }
          // Date field handling, check for date_api module to use date select widget
          // else fallback to textfield
          if($entity_form[$name]['#type'] == 'date_select') {
            if(module_exists('date_api')){
              $entity_form[$name]['#format'] = $info['widget']['format'];
            }
            else {
              $entity_form[$name]['#type'] = 'textfield';
            }
          }
          if(isset($entity_form['id'])) {
            unset($entity_form['id']);
          }
        } // end if isset widget
      } // end else not a drupal field
    } // end foreach

    return $entity_form;
  }
  
  public function entityFormValidate($entity_form, &$form_state) {
    
    $entity_type = $form_state['entity_type'];
    $field_name = $entity_form['#parents'][0] ;
    $ief_id = $entity_form['#ief_id'];
    $op = $entity_form['#op'];
     if($op == 'edit')
     {
      foreach($form_state['inline_entity_form'][$ief_id]['entities'] as $k => $entity_array) {
        if(isset($form_state['values'][$field_name]['und']['entities'][$k]['form'])){
          $entity = $entity_array['entity'];
          foreach ($form_state['values'][$field_name]['und']['entities'][$k]['form'] as $key => $value) {
            $entity->{$key} = $value;
          }
          field_attach_form_validate($entity_type, $entity, $entity_form, $form_state);
        }
      }
    }
    if($op == 'create') {
      $entity = $form_state['entity'];
      
      foreach ($form_state['values'][$field_name]['und']['form'] as $key => $value) {
        $entity->{$key} = $value;
      }
      field_attach_form_validate($entity_type, $entity, $entity_form, $form_state);
    }
    parent::entityFormValidate($entity_form, $form_state);
  }
  
  public function entityFormSubmit(&$entity_form, &$form_state) {
    $entity_type = $form_state['entity_type'];
    $field_name = $entity_form['#parents'][0] ;
    $ief_id = $entity_form['#ief_id'];
    $op = $entity_form['#op'];
    
    if($op == 'edit')
     {
      foreach($form_state['inline_entity_form'][$ief_id]['entities'] as $k => $entity_array) {
        if(isset($form_state['values'][$field_name]['und']['entities'][$k]['form'])) {
          $entity = $entity_array['entity'];
          foreach ($form_state['values'][$field_name]['und']['entities'][$k]['form'] as $key => $value) {
            $entity->{$key} = $value;
          }
          if ($entity->is_new = isset($entity->is_new) ? $entity->is_new : 0) {
            $entity->created = time();
          }
          $entity->changed = time();
        
          $wrapper = entity_metadata_wrapper($entity_type, $entity);
          field_attach_submit($entity_type, $entity, $entity_form, $form_state);
          $wrapper->save();
        }
      }
    }
    
    if($op == 'create') {

      $entity = $form_state['entity'];
      foreach ($form_state['values'][$field_name]['und']['form'] as $key => $value) {
        $entity->{$key} = $value;
      }
      if ($entity->is_new = isset($entity->is_new) ? $entity->is_new : 0) {
        $entity->created = time();
      }
      $entity->changed = time();

      $wrapper = entity_metadata_wrapper($entity_type, $entity);
      field_attach_submit($entity_type, $entity, $entity_form, $form_state);
      $wrapper->save();
    }
    // Add in created and changed times.
    
    parent::entityFormSubmit($entity_form, $form_state);
  }

}
