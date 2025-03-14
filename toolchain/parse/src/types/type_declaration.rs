use crate::matcher as m;
use combine::{choice, optional, Parser, Stream};
use lang::ast;

fn type_alias<T>() -> impl Parser<T, Output = ast::raw::TypeDeclaration>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::terminated((
        m::keyword("type"),
        m::binding(),
        m::symbol('='),
        super::type_expression::type_expression(),
    ))
    .map(|((_, start), binding, _, value)| {
        let end = value.0.range();
        let range = &start + end;

        ast::raw::TypeDeclaration::raw(ast::TypeDeclaration::type_alias(binding, value), range)
    })
}

fn enumerated<T>() -> impl Parser<T, Output = ast::raw::TypeDeclaration>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::terminated((
        m::keyword("enum"),
        m::binding(),
        m::closure(m::surround_by(
            crate::declaration::enumerated::variant(),
            || m::symbol('|'),
        )),
    ))
    .map(|((_, start), binding, (variants, end))| {
        let range = &start + &end;

        ast::raw::TypeDeclaration::raw(ast::TypeDeclaration::enumerated(binding, variants), range)
    })
}

fn view<T>() -> impl Parser<T, Output = ast::raw::TypeDeclaration>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::terminated((
        m::keyword("view"),
        m::binding(),
        super::type_expression::type_expression(),
    ))
    .map(|((_, start), binding, attributes)| {
        let range = &start + attributes.0.range();

        ast::raw::TypeDeclaration::raw(ast::TypeDeclaration::view(binding, attributes), range)
    })
}

fn function<T>() -> impl Parser<T, Output = ast::raw::TypeDeclaration>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::terminated((
        m::keyword("func"),
        m::binding(),
        m::lambda(
            optional(m::tuple(super::type_expression::type_expression())),
            super::type_expression::type_expression(),
        ),
    ))
    .map(|((_, start), binding, (parameters, result))| {
        let range = &start + result.0.range();

        ast::raw::TypeDeclaration::raw(
            ast::TypeDeclaration::function(
                binding,
                parameters.map(|x| x.0).unwrap_or_default(),
                result,
            ),
            range,
        )
    })
}

fn module<T, P>(parser: P) -> impl Parser<T, Output = ast::raw::TypeDeclaration>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::TypeModule>,
{
    m::terminated((m::keyword("module"), m::binding(), m::closure(parser))).map(
        |((_, start), binding, (module, end))| {
            let range = &start + &end;

            ast::raw::TypeDeclaration::raw(ast::TypeDeclaration::module(binding, module), range)
        },
    )
}

pub fn type_declaration<T, P, F>(parser: F) -> impl Parser<T, Output = ast::raw::TypeDeclaration>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::TypeModule>,
    F: Fn() -> P,
{
    choice((
        type_alias(),
        enumerated(),
        view(),
        function(),
        module(parser()),
    ))
}

#[cfg(test)]
mod tests {
    use combine::{stream::position::Stream, EasyParser};

    fn parse(s: &str) -> crate::Result<lang::ast::raw::TypeDeclaration> {
        super::type_declaration(crate::types::type_module::type_module).easy_parse(Stream::new(s))
    }

    #[test]
    fn type_alias() {
        let ast = parse("type foo = nil").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn enumerated() {
        let ast = parse("enum foo { Fizz | Buzz(integer) }").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn view() {
        let ast = parse("view Foo { bar: nil, fizz?: boolean }").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn function_no_parameters() {
        let ast = parse("func foo -> string;").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn function_with_parameters() {
        let ast = parse("func foo (integer, boolean) -> string;").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn empty_module() {
        let ast = parse("module foo {}").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn module() {
        let ast = parse(
            "module foo {
  type Bar = nil;
}",
        )
        .unwrap()
        .0;

        insta::assert_debug_snapshot!(ast);
    }
}
