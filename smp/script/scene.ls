(local make_box (fun (do
  (local entity (group_entity_create))
  (local transform (entity.require_transform))
  (local primitive (entity.add_primitive))
  (local light (entity.add_primitive))
  (transform.move
    (vec
      (rand_range -8 8)
      (rand_range -8 8)
      (rand_range 2 8)))
  (transform.rotate (vec 0 1 0) (* (- (rand) 0.5) 5))
  ; Add rigid body
  (entity.require_rigidbody|.add_velocity (vec (rand_range -2 2) (rand_range -2 2) (rand_range 1 2)))
  ; Add collider
  (if (< (rand) 0.0)
    (do
      (primitive.material|.parent "material/sphere.ls")
      (entity.require_collider|.sphere 0.5)
    )
    (do
      (primitive.material|.parent "material/box.ls")
      (entity.require_collider|.box (vec 0.5 0.5 0.5))
    )
  )
  (local color (rand_color))
  ; Geometry
  (primitive.scale (vec 0.5 0.5 0.5))
  (primitive.material|.color 'color color)
  ; Light
  (light.material|.parent "material/pointlight.ls")
  (light.material|.color 'color color)
  (light.material|.scalar 'intensity 4)
  (light.scale 32)
)))

(local make_static_box (fun position size (do
  (local entity (group_entity_create))
  (entity.require_transform|.move position)
  (entity.require_collider|.box size)
  (entity.require_primitive|.scale size)
  (entity.require_primitive|.material|.parent "material/box.ls")
)))

(local make_cage (fun (do
  (local size 20)
  (local nsize (- size))
  (local half (/ size 2))
  (local nhalf (- half))
  (make_static_box (vec 0 0 0) (vec half half 0.5))
  (make_static_box (vec half 0 half) (vec 0.5 half half))
  (make_static_box (vec nhalf 0 half) (vec 0.5 half half))
  (make_static_box (vec 0 half half) (vec half 0.5 half))
  (make_static_box (vec 0 nhalf 1) (vec half 0.5 1))
)))

(local make_mesh (fun mat pos (do
  (local entity (group_entity_create))
  (set last_mesh entity)
  (local transform (entity.require_transform))
  (local primitive (entity.require_primitive))
  (transform.move pos)
  (primitive.material|.parent mat)
  entity
)))

(group_entity_create|.add_script|.load "script/sink.ls")

; Make ambient light
(local amblight_entity (group_entity_create))
(amblight_entity.require_primitive|.material|.parent "material/ssao.ls")
(amblight_entity.require_primitive|.scale 99999)
(amblight_entity.require_primitive|.material|.color 'color (color 0.2 0.2 0.2))

; Make directional light
(local dirlight_entity (group_entity_create))
(dirlight_entity.require_transform|.rotate (vec -1 0 1) 1)
(dirlight_entity.require_primitive|.material|.parent "material/dirlight.ls")
(dirlight_entity.require_primitive|.scale 99999)
(dirlight_entity.require_primitive|.material|.scalar 'intensity 2)

; Make sky
(local sky_entity (group_entity_create))
(sky_entity.require_primitive|.material|.parent "material/sky.ls")
(sky_entity.require_primitive|.scale 99999)

; Make terrain
(local terrain_entity (group_entity_create))
(terrain_entity.require_transform|.move (vec -20 0 0))
(terrain_entity.require_primitive|.scale (vec 10 10 4))
(terrain_entity.require_primitive|.material|.parent "material/terrain.ls")

(make_cage)

(make_mesh "material/smartphone.ls" (vec -16 -20 5))
(make_mesh "material/jerrican.ls" (vec 10 -16 5))
(make_mesh "material/bush.ls" (vec -16 -28 0))

(make_mesh "material/DamagedHelmet.glb" (vec 20 0 10))
(last_mesh.require_primitive|.scale 10)
(last_mesh.require_transform|.rotate_absolute (vec 0 0 1) 3.14)

(local sprite (group_entity_create))
(sprite.require_transform|.move (vec -9.4 0 5))
(sprite.require_transform|.rotate (vec 0 0 1) 1.57)
(sprite.require_primitive|.material|.parent "material/sprite.ls")
(sprite.require_primitive|.material|.texture 'tex "texture/bush.png?comp=bc3")
(sprite.require_primitive|.scale (vec 5))

; Create all boxes
(local box_count 16)
(while (< 0 box_count) (do
  (make_box)
  (set box_count (- box_count 1))
))

;(group_entity_create|.add_group|.level_script "scene/test.glb")
