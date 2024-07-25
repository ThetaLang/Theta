#pragma once

#include <vector>
#include <deque>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <memory>
#include <filesystem>
#include <binaryen-c.h>

using namespace std;

namespace Theta {
    class StandardLibrary {
        public:
            static void registerFunctions(BinaryenModuleRef &module) {
                registerMathPow(module);
            }

        private:
            // FIXME: This won't work for negative values of an exponent like 10**-5. We need to check the sign and do division instead
            // if the exponent is negative. It also won't work for negative base values. If the base is negative we need to multiply times
            // the absolute value of the base in order to get the correct answer. It also wont work for floating point exponents.
            static void registerMathPow(BinaryenModuleRef &module) {
                /*
                (func (export "Theta.Math.pow") (param $base i32) (param $exp i32) (result i32) (local $res i32)
                  i32.const 1
                  local.set $res ;; 2

                  (loop $powLoop
                  	local.get $base ;; 0
                    	local.get $res ;; 2
                    	i32.mul
                    	local.set $res ;; 2

                    	local.get $exp ;; 1
                    	i32.const 1
                    	i32.sub
                    	local.set $exp ;; 1

                    	local.get $exp ;; 1
                    	i32.const 0
                    	i32.ne
                    	br_if $powLoop
                  )

                  local.get $res ;; 2
              )
                */

                BinaryenExpressionRef loopExpressions[] = {
                    // Multiply base * result
                    BinaryenLocalSet(
                        module,
                        2,
                        BinaryenBinary(
                            module,
                            BinaryenMulInt64(),
                            BinaryenLocalGet(module, 0, BinaryenTypeInt64()),
                            BinaryenLocalGet(module, 2, BinaryenTypeInt64())
                        )
                    ),
                    // Subtract 1 from exponent
                    BinaryenLocalSet(
                        module,
                        1,
                        BinaryenBinary(
                            module,
                            BinaryenSubInt64(),
                            BinaryenLocalGet(module, 1, BinaryenTypeInt64()),
                            BinaryenConst(module, BinaryenLiteralInt64(1))
                        )
                    ),
                    // Check if exponent is 0. If not, loop again
                    BinaryenBreak(
                        module,
                        "powLoop",
                        BinaryenBinary(
                            module,
                            BinaryenNeInt64(),
                            BinaryenLocalGet(module, 1, BinaryenTypeInt64()),
                            BinaryenConst(module, BinaryenLiteralInt64(0))
                        ),
                        NULL
                    )
                };

                BinaryenExpressionRef expressions[] = {
                    // Set the result to 1 first
                    BinaryenLocalSet(
                        module,
                        2,
                        BinaryenConst(module, BinaryenLiteralInt64(1))
                    ),
                    // Loop and multiply the base times itself as many times as needed
                    BinaryenLoop(
                        module,
                        "powLoop",
                        BinaryenBlock(
                            module,
                            NULL,
                            loopExpressions,
                            3,
                            BinaryenTypeNone()
                        )
                    ),
                    // Put the result on the stack so it gets returned
                    BinaryenLocalGet(module, 2, BinaryenTypeInt64())
                };

                BinaryenFunctionRef powFn = BinaryenAddFunction(
                    module,
                    "Theta.Math.pow",
                    BinaryenTypeCreate((BinaryenType[]){ BinaryenTypeInt64(), BinaryenTypeInt64() }, 2),
                    BinaryenTypeInt64(),
                    (BinaryenType[]){ BinaryenTypeInt64() },
                    1,
                    BinaryenBlock(
                        module,
                        NULL,
                        expressions,
                        3,
                        BinaryenTypeInt64()
                    )
                );
            }
    };
}
