use crate::matcher as m;
use combine::{attempt, choice, parser, sep_end_by, Parser, Stream};
use lang::ast;

fn primitive<T>() -> impl Parser<T, Output = ast::raw::TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    fn bind<U>(
        s: &'static str,
        f: impl Fn() -> ast::TypeExpression<ast::raw::Binding, ast::raw::TypeExpression>,
    ) -> impl Parser<U, Output = ast::raw::TypeExpression>
    where
        U: Stream<Token = char>,
        U::Position: m::Position,
    {
        m::keyword(s).map(move |(_, range)| ast::raw::TypeExpression::raw(f(), range))
    }

    choice((
        bind("nil", || {
            ast::TypeExpression::Primitive(ast::TypePrimitive::Nil)
        }),
        bind("boolean", || {
            ast::TypeExpression::Primitive(ast::TypePrimitive::Boolean)
        }),
        bind("integer", || {
            ast::TypeExpression::Primitive(ast::TypePrimitive::Integer)
        }),
        bind("float", || {
            ast::TypeExpression::Primitive(ast::TypePrimitive::Float)
        }),
        bind("string", || {
            ast::TypeExpression::Primitive(ast::TypePrimitive::String)
        }),
        bind("style", || {
            ast::TypeExpression::Primitive(ast::TypePrimitive::Style)
        }),
        bind("element", || {
            ast::TypeExpression::Primitive(ast::TypePrimitive::Element)
        }),
    ))
}

fn identifier<T>() -> impl Parser<T, Output = ast::raw::TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::standard_identifier()
        .map(|(x, range)| ast::raw::TypeExpression::raw(ast::TypeExpression::Identifier(x), range))
}

fn group<T, P>(parser: P) -> impl Parser<T, Output = ast::raw::TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::TypeExpression>,
{
    m::between(m::symbol('('), m::symbol(')'), parser).map(|(inner, range)| {
        ast::raw::TypeExpression::raw(ast::TypeExpression::Group(Box::new(inner)), range)
    })
}

fn property_access<T, P>(parser: P) -> impl Parser<T, Output = ast::raw::TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::TypeExpression>,
{
    m::folding(
        parser,
        m::symbol('.').with(m::standard_identifier()),
        |lhs, (rhs, end)| {
            let range = lhs.0.range() + &end;
            ast::raw::TypeExpression::raw(
                ast::TypeExpression::PropertyAccess(Box::new(lhs), rhs),
                range,
            )
        },
    )
}

fn object<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = ast::raw::TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::TypeExpression>,
{
    let required_entry = (m::binding(), m::symbol(':'), parser())
        .map(|(binding, _, x)| ast::ObjectTypeExpressionEntry::Required(binding, x));

    let optional_entry = (m::binding(), m::glyph("?:"), parser())
        .map(|(binding, _, x)| ast::ObjectTypeExpressionEntry::Optional(binding, x));

    let spread_entry =
        (m::glyph("..."), parser()).map(|(_, x)| ast::ObjectTypeExpressionEntry::Spread(x));

    m::closure(sep_end_by(
        choice((
            attempt(required_entry),
            attempt(optional_entry),
            spread_entry,
        )),
        m::symbol(','),
    ))
    .map(|(entries, range)| {
        ast::raw::TypeExpression::raw(ast::TypeExpression::Object(entries), range)
    })
}

fn function<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = ast::raw::TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::TypeExpression>,
{
    m::lambda(m::tuple(parser()), parser()).map(|((parameters, start), result)| {
        let range = &start + result.0.range();
        ast::raw::TypeExpression::raw(
            ast::TypeExpression::Function(parameters, Box::new(result)),
            range,
        )
    })
}

fn type_expression_2<T>() -> impl Parser<T, Output = ast::raw::TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    choice((
        function(type_expression),
        group(type_expression()),
        object(type_expression),
        primitive(),
        identifier(),
    ))
}

fn type_expression_1<T>() -> impl Parser<T, Output = ast::raw::TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    property_access(type_expression_2())
}

// TODO: use this for lists ([], [][][])
fn type_expression_0<T>() -> impl Parser<T, Output = ast::raw::TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    type_expression_1()
}

parser! {
    pub fn type_expression[T]()(T) -> ast::raw::TypeExpression
    where
        [T: Stream<Token = char>, T::Position: m::Position]
    {
        type_expression_0()
    }
}

#[cfg(test)]
mod tests {
    use combine::{stream::position::Stream, EasyParser};

    fn parse(s: &str) -> crate::Result<lang::ast::raw::TypeExpression> {
        super::type_expression().easy_parse(Stream::new(s))
    }

    #[test]
    fn nil() {
        let ast = parse("nil").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn boolean() {
        let ast = parse("boolean").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn integer() {
        let ast = parse("integer").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn float() {
        let ast = parse("float").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn string() {
        let ast = parse("string").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn style() {
        let ast = parse("style").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn element() {
        let ast = parse("element").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn identifier() {
        let ast = parse("foo").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn group() {
        let ast = parse("(nil)").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn function() {
        let ast = parse("(nil, boolean) -> nil").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn function_empty_parameters() {
        let ast = parse("() -> nil").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn property_access() {
        let ast = parse("nil.foo").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn empty_object() {
        let ast = parse("{}").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn object() {
        let ast = parse(
            "{
  foo: integer,
  bar?: boolean,
  ...fizz
}",
        )
        .unwrap()
        .0;

        insta::assert_debug_snapshot!(ast);
    }
}
