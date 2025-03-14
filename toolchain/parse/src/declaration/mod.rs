mod constant;
pub mod enumerated;
mod function;
mod module;
pub mod parameter;
pub mod storage;
mod type_alias;
mod view;

use crate::matcher as m;
use combine::{choice, parser, Stream};

parser! {
    pub fn declaration[T]()(T) -> lang::ast::raw::Declaration
    where
        [T: Stream<Token = char>, T::Position: m::Position]
    {
        choice((
            type_alias::type_alias(),
            constant::constant(),
            enumerated::enumerated(),
            function::function(),
            view::view(),
            module::module(),
        ))
    }
}

#[cfg(test)]
mod tests {
    use combine::{eof, stream::position::Stream, EasyParser, Parser};

    fn parse(s: &str) -> crate::Result<lang::ast::raw::Declaration> {
        super::declaration().skip(eof()).easy_parse(Stream::new(s))
    }

    #[test]
    fn type_alias() {
        let ast = parse("type foo = nil;").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn constant() {
        let ast = parse("const foo = nil;").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn enumerated() {
        let ast = parse("enum foo { Fizz(nil) }").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn enumerated_empty_parameters() {
        let ast = parse("enum foo { Fizz() }").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn enumerated_no_parameters() {
        let ast = parse("enum foo { Fizz }").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn function() {
        let ast = parse("func foo -> nil;").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn function_result_typedef() {
        let ast = parse("func foo: nil -> nil;").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn function_empty_parameters() {
        let ast = parse("func foo() -> nil;").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn function_empty_parameters_result_typedef() {
        let ast = parse("func foo(): nil -> nil;").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn view() {
        let ast = parse("view foo -> nil;").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn view_empty_arguments() {
        let ast = parse("view foo {} -> nil;").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn module_empty() {
        let ast = parse("module foo {}").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn module() {
        let ast = parse("module foo { const bar = nil; }").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }
}
