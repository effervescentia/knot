pub mod import;

use crate::{
    ast::{
        import::Import,
        module::{Module, ModuleNode},
    },
    common::position::Decrement,
    parser::declaration,
};
use combine::{choice, many, Parser, Stream};
use std::fmt::Debug;

#[derive(Debug, PartialEq)]
enum Entry<D> {
    Import(Import),
    Declaration(D),
}

pub fn module<T>() -> impl Parser<T, Output = ModuleNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    many::<Vec<_>, _, _>(choice((
        import::import().map(Entry::Import),
        declaration::declaration().map(Entry::Declaration),
    )))
    .map(|entries| {
        ModuleNode::raw(
            entries
                .into_iter()
                .fold(Module::new(vec![], vec![]), |mut acc, el| {
                    match el {
                        Entry::Import(import) => {
                            acc.imports.push(import);
                        }
                        Entry::Declaration(declaration) => {
                            acc.declarations.push(declaration);
                        }
                    }

                    acc
                }),
        )
    })
}

#[cfg(test)]
mod tests {
    use crate::{
        ast::{
            expression::{Expression, Primitive},
            import::{Import, Source},
            ksx::KSX,
            module::{Module, ModuleNode},
            operator::BinaryOperator,
            parameter::Parameter,
            statement::Statement,
            type_expression::TypeExpression,
        },
        parser::{CharStream, ParseResult},
        test::fixture as f,
    };
    use combine::{eof, stream::position::Stream, EasyParser, Parser};

    fn parse(s: &str) -> ParseResult<ModuleNode<CharStream, ()>> {
        super::module().skip(eof()).easy_parse(Stream::new(s))
    }

    #[test]
    fn module_empty() {
        assert_eq!(parse("").unwrap().0, f::n::mr(Module::new(vec![], vec![])));
    }

    #[test]
    fn module_import() {
        assert_eq!(
            parse("use @/foo;").unwrap().0,
            f::n::mr(Module::new(
                vec![Import::new(Source::Root, vec![String::from("foo")], None)],
                vec![]
            ))
        );
    }

    #[test]
    fn module_declaration() {
        assert_eq!(
            parse("const foo = nil;").unwrap().0,
            f::n::mr(Module::new(
                vec![],
                vec![f::n::dr(
                    f::a::const_(
                        "foo",
                        None,
                        f::n::xr(Expression::Primitive(Primitive::Nil), ((1, 13), (1, 15)))
                    ),
                    ((1, 1), (1, 15))
                )]
            ))
        );
    }

    #[test]
    fn multiple_declarations() {
        let source = "type MyTypeAlias = nil;

enum MyEnum =
  | First
  | Second(boolean, style);

const MY_CONST: string = \"hello, world!\";

func my_func(first, second: integer, third = true): boolean -> first > second || third;

view MyView(inner: element = <div />) -> {
  let value = 123 + 45.67;

  <>
    <h1>Welcome!</h1>
    <main>{value}: {inner}</main>
  </>;
};

module my_module {
  const MY_STYLE = style {
    color: \"red\",
    display: \"block\",
  };
}";

        assert_eq!(
            parse(source).unwrap().0,
            f::n::mr(Module::new(
                vec![],
                vec![
                    f::n::dr(
                        f::a::type_(
                            "MyTypeAlias",
                            f::n::txr(TypeExpression::Nil, ((1, 20), (1, 22)))
                        ),
                        ((1, 1), (1, 22))
                    ),
                    f::n::dr(
                        f::a::enum_(
                            "MyEnum",
                            vec![
                                (String::from("First"), vec![]),
                                (
                                    String::from("Second"),
                                    vec![
                                        f::n::txr(TypeExpression::Boolean, ((5, 12), (5, 18))),
                                        f::n::txr(TypeExpression::Style, ((5, 21), (5, 25))),
                                    ]
                                ),
                            ]
                        ),
                        ((3, 1), (5, 26))
                    ),
                    f::n::dr(
                        f::a::const_(
                            "MY_CONST",
                            Some(f::n::txr(TypeExpression::String, ((7, 17), (7, 22)))),
                            f::n::xr(
                                Expression::Primitive(Primitive::String(String::from(
                                    "hello, world!"
                                ))),
                                ((7, 26), (7, 40))
                            )
                        ),
                        ((7, 1), (7, 40))
                    ),
                    f::n::dr(
                        f::a::func_(
                            "my_func",
                            vec![
                                f::n::pr(
                                    Parameter::new(String::from("first"), None, None),
                                    ((9, 14), (9, 18))
                                ),
                                f::n::pr(
                                    Parameter::new(
                                        String::from("second"),
                                        Some(f::n::txr(
                                            TypeExpression::Integer,
                                            ((9, 29), (9, 35))
                                        )),
                                        None
                                    ),
                                    ((9, 21), (9, 35))
                                ),
                                f::n::pr(
                                    Parameter::new(
                                        String::from("third"),
                                        None,
                                        Some(f::n::xr(
                                            Expression::Primitive(Primitive::Boolean(true)),
                                            ((9, 46), (9, 49))
                                        ))
                                    ),
                                    ((9, 38), (9, 49))
                                )
                            ],
                            Some(f::n::txr(TypeExpression::Boolean, ((9, 53), (9, 59)))),
                            f::n::xr(
                                Expression::BinaryOperation(
                                    BinaryOperator::Or,
                                    Box::new(f::n::xr(
                                        Expression::BinaryOperation(
                                            BinaryOperator::GreaterThan,
                                            Box::new(f::n::xr(
                                                Expression::Identifier(String::from("first")),
                                                ((9, 64), (9, 68))
                                            )),
                                            Box::new(f::n::xr(
                                                Expression::Identifier(String::from("second")),
                                                ((9, 72), (9, 77))
                                            ))
                                        ),
                                        ((9, 64), (9, 77))
                                    )),
                                    Box::new(f::n::xr(
                                        Expression::Identifier(String::from("third")),
                                        ((9, 82), (9, 86))
                                    ))
                                ),
                                ((9, 64), (9, 86))
                            )
                        ),
                        ((9, 1), (9, 86))
                    ),
                    f::n::dr(
                        f::a::view(
                            "MyView",
                            vec![f::n::pr(
                                Parameter::new(
                                    String::from("inner"),
                                    Some(f::n::txr(TypeExpression::Element, ((11, 20), (11, 26)))),
                                    Some(f::n::xr(
                                        Expression::KSX(Box::new(f::n::kxr(
                                            KSX::ClosedElement(String::from("div"), vec![]),
                                            ((11, 30), (11, 36))
                                        ))),
                                        ((11, 30), (11, 36))
                                    ))
                                ),
                                ((11, 13), (11, 36))
                            )],
                            f::n::xr(
                                Expression::Closure(vec![
                                    f::n::sr(
                                        Statement::Variable(
                                            String::from("value"),
                                            f::n::xr(
                                                Expression::BinaryOperation(
                                                    BinaryOperator::Add,
                                                    Box::new(f::n::xr(
                                                        Expression::Primitive(Primitive::Integer(
                                                            123
                                                        )),
                                                        ((12, 15), (12, 17))
                                                    ),),
                                                    Box::new(f::n::xr(
                                                        Expression::Primitive(Primitive::Float(
                                                            45.67, 2
                                                        )),
                                                        ((12, 21), (12, 25))
                                                    ),),
                                                ),
                                                ((12, 15), (12, 25))
                                            )
                                        ),
                                        ((12, 3), (12, 25))
                                    ),
                                    f::n::sr(
                                        Statement::Expression(f::n::xr(
                                            Expression::KSX(Box::new(f::n::kxr(
                                                KSX::Fragment(vec![
                                                    f::n::kxr(
                                                        KSX::OpenElement(
                                                            String::from("h1"),
                                                            vec![],
                                                            vec![f::n::kxr(
                                                                KSX::Text(String::from("Welcome!")),
                                                                ((15, 9), (15, 16))
                                                            )],
                                                            String::from("h1")
                                                        ),
                                                        ((15, 5), (15, 21))
                                                    ),
                                                    f::n::kxr(
                                                        KSX::OpenElement(
                                                            String::from("main"),
                                                            vec![],
                                                            vec![
                                                                f::n::kxr(
                                                                    KSX::Inline(f::n::xr(
                                                                        Expression::Identifier(
                                                                            String::from("value")
                                                                        ),
                                                                        ((16, 12), (16, 16))
                                                                    )),
                                                                    ((16, 11), (16, 17))
                                                                ),
                                                                f::n::kxr(
                                                                    KSX::Text(String::from(": ")),
                                                                    ((16, 18), (16, 19))
                                                                ),
                                                                f::n::kxr(
                                                                    KSX::Inline(f::n::xr(
                                                                        Expression::Identifier(
                                                                            String::from("inner")
                                                                        ),
                                                                        ((16, 21), (16, 25))
                                                                    )),
                                                                    ((16, 20), (16, 26))
                                                                )
                                                            ],
                                                            String::from("main")
                                                        ),
                                                        ((16, 5), (16, 33))
                                                    )
                                                ]),
                                                ((14, 3), (17, 5))
                                            ))),
                                            ((14, 3), (17, 5))
                                        )),
                                        ((14, 3), (17, 5))
                                    )
                                ]),
                                ((11, 42), (18, 1))
                            )
                        ),
                        ((11, 1), (18, 1))
                    ),
                    f::n::dr(
                        f::a::module(
                            "my_module",
                            f::n::mr(Module::new(
                                vec![],
                                vec![f::n::dr(
                                    f::a::const_(
                                        "MY_STYLE",
                                        None,
                                        f::n::xr(
                                            Expression::Style(vec![
                                                (
                                                    String::from("color"),
                                                    f::n::xr(
                                                        Expression::Primitive(Primitive::String(
                                                            String::from("red")
                                                        )),
                                                        ((22, 12), (22, 16))
                                                    )
                                                ),
                                                (
                                                    String::from("display"),
                                                    f::n::xr(
                                                        Expression::Primitive(Primitive::String(
                                                            String::from("block")
                                                        )),
                                                        ((23, 14), (23, 20))
                                                    )
                                                )
                                            ]),
                                            ((21, 20), (24, 3))
                                        )
                                    ),
                                    ((21, 3), (24, 3))
                                )]
                            ))
                        ),
                        ((20, 1), (25, 1))
                    )
                ]
            ))
        );
    }
}
