(module
  (import "console" "log" (func $log (param stringref))) ;; TODO: Remove this
  (memory $0 1 10)
  (func $Theta.Function.populateClosure (param $closure_mem_addr i32) (param $param_addr i32) (local $arity i32)
    (local.set $arity ;; Load the closure arity
      (i32.load 
        (i32.add
          (local.get $closure_mem_addr)
          (i32.const 4)
        )  
      )
    )
    (i32.store offset=4 ;; Update the arity 
      (local.get $closure_mem_addr)
      (i32.sub
        (local.get $arity)
        (i32.const 1)
      )
    )
    (i32.store offset=4 ;; Store param ptr into memory
      (i32.add
        (local.get $closure_mem_addr)
        (i32.mul
          (local.get $arity)
          (i32.const 4)
        )
      )
      (local.get $param_addr)
    )
  )
)
