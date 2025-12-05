(module
  (import "env" "print_string" (func $print_string( param i32 )))
  (func (export "AddInt")
    (param $value_1 i32) (param $value_2 i32)
    (result i32)
      (call $print_string (global.get $string_len))
      (i32.add 
        (i32.add (local.get $value_1) (local.get $value_2)) 
        (i32.const 4)
      )
  )
)

(; (module ;)
(;   (func (export "AddInt") ;)
(;     (param $value_1 i32) (param $value_2 i32) ;)
(;     (result i32) ;)
(;       local.get $value_1 ;)
(;       local.get $value_2 ;)
(;       i32.add ;)
(;   ) ;)
(; ) ;)

;; clang --target=wasm32 -O3 -flto -nostdlib -Wl,--no-entry -Wl,--export-all -Wl,--lto-O3 -o add.wasm add.c
