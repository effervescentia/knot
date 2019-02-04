open Core;
open KnotLex.Core;

let mutate = x =>
  fun
  | LazyStream.Cons((_, mutator), _) as res => {
      mutator(x);
      Some(((), res));
    }
  | LazyStream.Nil => None;

let push = (x, input) => mutate(PushContext(x), input);
let pop = input => mutate(PopContext, input);
let swap = (x, input) => mutate(SwapContext(x), input);
let no_op = input => mutate(NoOp, input);
