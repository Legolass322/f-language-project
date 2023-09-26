(func binaryToDecimal (binary)
  (return 
    (foldl 
      (lambda (value item)
		    (return (plus (times 2 value) item))
      )
      0 
      binary
    )
  )
)

(binaryToDecimal (1 0 0 1 0))