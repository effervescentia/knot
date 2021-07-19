/* type system */

type wrapper_t = [
  | `Weak(int, int)
  | `Strong
];

type primitive_t =
  | Boolean
  | Number
  | String;
