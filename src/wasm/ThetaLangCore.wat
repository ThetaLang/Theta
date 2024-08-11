(module
  (memory $0 1 10)
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
