use crate::expression::{self, Expression};
use crate::module::Module;
use crate::types::type_expression::{self, TypeExpression};
use crate::{matcher as m, module};
use combine::{between, choice, many1, optional, parser, sep_end_by, value, Parser, Stream};

#[derive(Clone, Debug, PartialEq)]
pub enum ImportSource {
    Root,
    Local,
    External(String),
}

#[derive(Clone, Debug, PartialEq)]
pub enum ImportTarget {
    Named(String),
    Module,
}

#[derive(Clone, Debug, PartialEq)]
pub enum Visibility {
    Public,
    Private,
}

#[derive(Clone, Debug, PartialEq)]
pub struct Storage(Visibility, String);

impl Storage {
    pub fn new(visibility: Visibility, name: String) -> Storage {
        Storage(visibility, name)
    }
}

#[derive(Debug, PartialEq)]
pub struct Parameter {
    name: String,
    value_type: Option<TypeExpression>,
    default_value: Option<Expression>,
}

#[derive(Debug, PartialEq)]
pub enum Declaration {
    Type {
        name: Storage,
        value: TypeExpression,
    },
    Constant {
        name: Storage,
        value_type: Option<TypeExpression>,
        value: Expression,
    },
    Enumerated {
        name: Storage,
        variants: Vec<(String, Vec<TypeExpression>)>,
    },
    Function {
        name: Storage,
        parameters: Vec<Parameter>,
        body_type: Option<TypeExpression>,
        body: Expression,
    },
    View {
        name: Storage,
        attributes: Vec<Parameter>,
        body: Expression,
    },
    Module {
        name: Storage,
        value: Module,
    },
}

fn storage<T>(keyword: &'static str) -> impl Parser<T, Output = Storage>
where
    T: Stream<Token = char>,
{
    (
        m::keyword("priv")
            .with(value(Visibility::Private))
            .or(value(Visibility::Public)),
        m::keyword(keyword),
        m::standard_identifier(),
    )
        .map(|(visibility, _, name)| Storage(visibility, name))
}

fn typedef<T>() -> impl Parser<T, Output = Option<TypeExpression>>
where
    T: Stream<Token = char>,
{
    optional(m::symbol(':').with(type_expression::type_expression()))
}

fn parameter<T>() -> impl Parser<T, Output = Parameter>
where
    T: Stream<Token = char>,
{
    (
        m::standard_identifier(),
        typedef(),
        optional(m::symbol('=').with(expression::expression())),
    )
        .map(|(name, value_type, default_value)| Parameter {
            name,
            value_type,
            default_value,
        })
}

fn type_<T>() -> impl Parser<T, Output = Declaration>
where
    T: Stream<Token = char>,
{
    m::terminated((
        storage("type"),
        m::symbol('='),
        type_expression::type_expression(),
    ))
    .map(|(name, _, value)| Declaration::Type { name, value })
}

fn constant<T>() -> impl Parser<T, Output = Declaration>
where
    T: Stream<Token = char>,
{
    m::terminated((
        storage("const"),
        typedef(),
        m::symbol('='),
        expression::expression(),
    ))
    .map(|(name, value_type, _, value)| Declaration::Constant {
        name,
        value_type,
        value,
    })
}

fn enumerated<T>() -> impl Parser<T, Output = Declaration>
where
    T: Stream<Token = char>,
{
    let variant = || {
        (
            m::symbol('|'),
            m::standard_identifier(),
            between(
                m::symbol('('),
                m::symbol(')'),
                sep_end_by(type_expression::type_expression(), m::symbol(',')),
            )
            .or(value(vec![])),
        )
            .map(|(_, name, parameters)| (name, parameters))
    };

    m::terminated((storage("enum"), m::symbol('='), many1(variant())))
        .map(|(name, _, variants)| Declaration::Enumerated { name, variants })
}

// func foo -> nil;
// func foo -> {};
// func foo -> { nil; };
// func foo: nil -> nil;
// func foo() -> nil;
// func foo(): nil -> nil;
// func foo(a, b: nil, c = 123) -> nil;

fn function<T>() -> impl Parser<T, Output = Declaration>
where
    T: Stream<Token = char>,
{
    m::terminated((
        storage("func"),
        optional(between(
            m::symbol('('),
            m::symbol(')'),
            sep_end_by(parameter(), m::symbol(',')),
        )),
        typedef(),
        m::glyph("->"),
        expression::expression(),
    ))
    .map(
        |(name, parameters, body_type, _, body)| Declaration::Function {
            name,
            parameters: parameters.unwrap_or(vec![]),
            body_type,
            body,
        },
    )
}

// view foo -> nil;
// view foo -> {};
// view foo -> { nil; };
// view foo: nil -> nil;
// view foo() -> nil;
// view foo(): nil -> nil;
// view foo(props) -> nil;
// view foo({a, b: nil, c = 123}) -> nil;

fn view<T>() -> impl Parser<T, Output = Declaration>
where
    T: Stream<Token = char>,
{
    m::terminated((
        storage("view"),
        optional(between(
            m::symbol('('),
            m::symbol(')'),
            sep_end_by(parameter(), m::symbol(',')),
        )),
        m::glyph("->"),
        expression::expression(),
    ))
    .map(|(name, attributes, _, body)| Declaration::View {
        name,
        attributes: attributes.unwrap_or(vec![]),
        body,
    })
}

fn module<T>() -> impl Parser<T, Output = Declaration>
where
    T: Stream<Token = char>,
{
    (
        storage("module"),
        between(m::symbol('{'), m::symbol('}'), module::module()),
    )
        .map(|(name, value)| Declaration::Module { name, value })
}

fn declaration_<T>() -> impl Parser<T, Output = Declaration>
where
    T: Stream<Token = char>,
{
    choice((
        type_(),
        constant(),
        enumerated(),
        function(),
        view(),
        module(),
    ))
}

parser! {
    pub fn declaration[T]()(T) -> Declaration
    where
        [T: Stream<Token = char>]
    {
        declaration_()
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        declaration::{self, Declaration, Storage, Visibility},
        expression::Expression,
        module::Module,
        primitive::Primitive,
        types::type_expression::TypeExpression,
    };
    use combine::Parser;

    #[test]
    fn type_() {
        let parse = |s| declaration::declaration().parse(s);

        assert_eq!(
            parse("type foo = nil;").unwrap().0,
            Declaration::Type {
                name: Storage(Visibility::Public, String::from("foo")),
                value: TypeExpression::Nil
            }
        );
    }

    #[test]
    fn constant() {
        let parse = |s| declaration::declaration().parse(s);

        assert_eq!(
            parse("const foo = nil;").unwrap().0,
            Declaration::Constant {
                name: Storage(Visibility::Public, String::from("foo")),
                value_type: None,
                value: Expression::Primitive(Primitive::Nil)
            }
        );
    }

    #[test]
    fn enumerated() {
        let parse = |s| declaration::declaration().parse(s);

        assert_eq!(
            parse("enum foo = | Fizz | Buzz(nil);").unwrap().0,
            Declaration::Enumerated {
                name: Storage(Visibility::Public, String::from("foo")),
                variants: vec![
                    (String::from("Fizz"), vec![]),
                    (String::from("Buzz"), vec![TypeExpression::Nil])
                ]
            }
        );
    }

    #[test]
    fn function() {
        let parse = |s| declaration::declaration().parse(s);

        assert_eq!(
            parse("func foo -> nil;").unwrap().0,
            Declaration::Function {
                name: Storage(Visibility::Public, String::from("foo")),
                parameters: vec![],
                body_type: None,
                body: Expression::Primitive(Primitive::Nil)
            }
        );
        assert_eq!(
            parse("func foo: nil -> nil;").unwrap().0,
            Declaration::Function {
                name: Storage(Visibility::Public, String::from("foo")),
                parameters: vec![],
                body_type: Some(TypeExpression::Nil),
                body: Expression::Primitive(Primitive::Nil)
            }
        );
        assert_eq!(
            parse("func foo() -> nil;").unwrap().0,
            Declaration::Function {
                name: Storage(Visibility::Public, String::from("foo")),
                parameters: vec![],
                body_type: None,
                body: Expression::Primitive(Primitive::Nil)
            }
        );
        assert_eq!(
            parse("func foo(): nil -> nil;").unwrap().0,
            Declaration::Function {
                name: Storage(Visibility::Public, String::from("foo")),
                parameters: vec![],
                body_type: Some(TypeExpression::Nil),
                body: Expression::Primitive(Primitive::Nil)
            }
        );
    }

    #[test]
    fn view() {
        let parse = |s| declaration::declaration().parse(s);

        assert_eq!(
            parse("view foo -> nil;").unwrap().0,
            Declaration::View {
                name: Storage(Visibility::Public, String::from("foo")),
                attributes: vec![],
                body: Expression::Primitive(Primitive::Nil)
            }
        );
        assert_eq!(
            parse("view foo() -> nil;").unwrap().0,
            Declaration::View {
                name: Storage(Visibility::Public, String::from("foo")),
                attributes: vec![],
                body: Expression::Primitive(Primitive::Nil)
            }
        );
    }

    #[test]
    fn module() {
        let parse = |s| declaration::declaration().parse(s);

        assert_eq!(
            parse("module foo {}").unwrap().0,
            Declaration::Module {
                name: Storage(Visibility::Public, String::from("foo")),
                value: Module::new(vec![], vec![])
            }
        );

        assert_eq!(
            parse("module foo { const bar = nil; }").unwrap().0,
            Declaration::Module {
                name: Storage(Visibility::Public, String::from("foo")),
                value: Module::new(
                    vec![],
                    vec![Declaration::Constant {
                        name: Storage(Visibility::Public, String::from("bar")),
                        value_type: None,
                        value: Expression::Primitive(Primitive::Nil)
                    }]
                )
            }
        );
    }
}
