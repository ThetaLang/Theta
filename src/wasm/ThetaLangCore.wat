(module
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
    (i32.store ;; Store param ptr into memory
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
  (func $Theta.Function.executeIndirect (param $fn_idx i32) (result i32) (local $arity i32)
    (local.set $arity
      (i32.load
        (i32.add
          (local.get $fn_idx)
          (i32.const 4)
        )
      )
    )
    (if (result i32)
      (i32.eqz (local.get $arity))
      (then
        (i32.const 1)
      )
      (else
        (local.get $fn_idx)
      )
    )
  )
)
