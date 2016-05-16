(local transform)
(local center-x (1920 / 2))
(local center-y (1080 / 2))

(local start '(do
	(set transform (transform-get entity))
	(mouse-set !center-x !center-y)
))
(local update '(do
	; Camera rotation
	(local mdelta (object
		'x ((mouse-x) - !center-x)
		'y ((mouse-y) - !center-y)))
	(transform-rotate transform 0 0 1 ((mdelta'x) * -0.01))
	(transform-rotate transform 1 0 0 ((mdelta'y) * -0.01))
	(mouse-set !center-x !center-y) ; Reset mouse cursor position
	; Camera movement
	(local movement (delta * 4))
	(if (key-pressed Z) (transform-move transform (vec 0 movement 0)))
	(if (key-pressed Q) (transform-move transform (vec (0.0 - movement) 0 0)))
	(if (key-pressed S) (transform-move transform (vec 0 (0.0 - movement) 0)))
	(if (key-pressed D) (transform-move transform (vec movement 0 0)))
))
