G21 ; set units to millimeters
M106 S204 ; turn fan on
M190 S50 ; wait for bed temperature to be reached
M104 S210 ; set temperature
G28 ; home all axes
G92 E0
M109 S210 ; wait for temperature to be reached
G90 ; use absolute coordinates
G92 E0
M82 ; use absolute distances for extrusion
G92 E0
M106 S204 ; turn fan on

G1 F7200
; add_brim : 	True
; add_raft : 	False
; bed_size_x_mm : 	200
; bed_size_y_mm : 	150
; bed_temp_degree_c : 	50
; brim_distance_to_print_mm : 	1
; brim_num_contours : 	4
; cover_thickness_mm_0 : 	1.2
; enable_active_temperature_control : 	False
; enable_fan : 	True
; enable_fan_first_layer : 	False
; enable_fit_single_path : 	True
; extruder_0 : 	0
; extruder_count : 	1
; extruder_degrees_per_sec_0 : 	2
; extruder_mix_count_0 : 	1
; extruder_mix_ratios_0 : 	<error>
; extruder_swap_at_location : 	False
; extruder_swap_location_x_mm : 	0
; extruder_swap_location_y_mm : 	0
; extruder_swap_retract_length_mm : 	6
; extruder_swap_retract_speed_mm_per_sec : 	20
; extruder_swap_zlift_mm : 	1
; extruder_temp_degree_c_0 : 	210
; fan_speed_percent : 	100
; fan_speed_percent_on_bridges : 	100
; filament_diameter_mm_0 : 	1.75
; filament_priming_mm_0 : 	4
; first_layer_print_speed_mm_per_sec : 	10
; flow_multiplier_0 : 	1
; gen_shield : 	False
; gen_supports : 	False
; gen_tower : 	False
; idle_extruder_temp_degree_c_0 : 	100
; infill_extruder_0 : 	0
; infill_percentage_0 : 	20
; infill_type_0 : 	Default
; nozzle_diameter_mm : 	0.4
; num_shells_0 : 	1
; path_width_speed_adjustement_exponent : 	3
; perimeter_print_speed_mm_per_sec : 	30
; priming_mm_per_sec : 	40
; print_perimeter_0 : 	True
; print_speed_mm_per_sec : 	40
; printer : 	prusai3
; process_thin_features : 	False
; raft_spacing : 	1
; seam_location_field : 	
; shield_distance_to_part_mm : 	2
; shield_num_contours : 	1
; speed_multiplier_0 : 	1
; support_anchor_diameter : 	7.2
; support_extruder : 	0
; support_flow_multiplier : 	1
; support_interdiction_field : 	
; support_max_bridge_len_mm : 	10
; support_pillar_cross_length : 	2.5
; support_print_speed_mm_per_sec : 	40
; support_spacing_min_mm : 	2
; thickening_ratio : 	1.25
; tower_brim_num_contours : 	12
; tower_side_x_mm : 	10
; tower_side_y_mm : 	15
; travel_avoid_top_covers : 	False
; travel_max_length_without_retract : 	20
; travel_speed_mm_per_sec : 	120
; travel_straight : 	False
; z_layer_height_mm : 	0.3
M106 S0
G1 F7200
; <layer>

