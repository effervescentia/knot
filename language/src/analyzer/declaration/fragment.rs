use crate::{
    analyzer::{context::NodeContext, fragment::Fragment, register::ToFragment},
    parser::{
        declaration::{self, Declaration},
        position::Decrement,
    },
};
use combine::Stream;
use std::fmt::Debug;

impl<T> ToFragment for declaration::NodeValue<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_fragment<'a>(&'a self) -> Fragment {
        Fragment::Declaration(match self {
            Declaration::TypeAlias { name, value } => Declaration::TypeAlias {
                name: name.clone(),
                value: *value.0.id(),
            },

            Declaration::Enumerated { name, variants } => Declaration::Enumerated {
                name: name.clone(),
                variants: variants
                    .into_iter()
                    .map(|(name, params)| {
                        (
                            name.clone(),
                            params
                                .into_iter()
                                .map(|x| *x.node().id())
                                .collect::<Vec<_>>(),
                        )
                    })
                    .collect::<Vec<_>>(),
            },

            Declaration::Constant {
                name,
                value_type,
                value,
            } => Declaration::Constant {
                name: name.clone(),
                value_type: value_type.as_ref().map(|x| *x.node().id()),
                value: *value.0.id(),
            },

            Declaration::Function {
                name,
                parameters,
                body_type,
                body,
            } => Declaration::Function {
                name: name.clone(),
                parameters: parameters
                    .into_iter()
                    .map(|x| *x.0.id())
                    .collect::<Vec<_>>(),
                body_type: body_type.as_ref().map(|x| *x.node().id()),
                body: *body.node().id(),
            },

            Declaration::View {
                name,
                parameters,
                body,
            } => Declaration::View {
                name: name.clone(),
                parameters: parameters
                    .into_iter()
                    .map(|x| *x.0.id())
                    .collect::<Vec<_>>(),
                body: *body.node().id(),
            },

            Declaration::Module { name, value } => Declaration::Module {
                name: name.clone(),
                value: *value.id(),
            },
        })
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, fragment::Fragment, register::ToFragment},
        parser::{
            declaration::parameter::Parameter,
            expression::{primitive::Primitive, Expression},
            module::{
                import::{Import, Source, Target},
                Module, ModuleNode,
            },
            types::type_expression::TypeExpression,
        },
        test::fixture as f,
    };

    #[test]
    fn type_alias() {
        assert_eq!(
            f::a::type_(
                "Foo",
                f::n::txc(TypeExpression::Nil, NodeContext::new(0, vec![0, 1]))
            )
            .to_fragment(),
            Fragment::Declaration(f::a::type_("Foo", 0))
        );
    }

    #[test]
    fn enumerated() {
        assert_eq!(
            f::a::enum_(
                "Foo",
                vec![(
                    String::from("Bar"),
                    vec![f::n::txc(
                        TypeExpression::Nil,
                        NodeContext::new(0, vec![0, 1])
                    )]
                )]
            )
            .to_fragment(),
            Fragment::Declaration(f::a::enum_("Foo", vec![(String::from("Bar"), vec![0])]))
        );
    }

    #[test]
    fn constant() {
        assert_eq!(
            f::a::const_(
                "FOO",
                Some(f::n::txc(
                    TypeExpression::Nil,
                    NodeContext::new(0, vec![0, 1])
                )),
                f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(1, vec![0, 1])
                )
            )
            .to_fragment(),
            Fragment::Declaration(f::a::const_("FOO", Some(0), 1))
        );
    }

    #[test]
    fn function() {
        assert_eq!(
            f::a::func_(
                "foo",
                vec![f::n::pc(
                    Parameter::new(
                        String::from("bar"),
                        Some(f::n::txc(
                            TypeExpression::Nil,
                            NodeContext::new(0, vec![0, 1])
                        )),
                        Some(f::n::xc(
                            Expression::Primitive(Primitive::Nil),
                            NodeContext::new(1, vec![0, 1])
                        )),
                    ),
                    NodeContext::new(2, vec![0, 1])
                )],
                Some(f::n::txc(
                    TypeExpression::Nil,
                    NodeContext::new(3, vec![0, 1])
                )),
                f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(4, vec![0, 1])
                )
            )
            .to_fragment(),
            Fragment::Declaration(f::a::func_("foo", vec![2], Some(3), 4))
        );
    }

    #[test]
    fn view() {
        assert_eq!(
            f::a::view(
                "Foo",
                vec![f::n::pc(
                    Parameter::new(
                        String::from("bar"),
                        Some(f::n::txc(
                            TypeExpression::Nil,
                            NodeContext::new(0, vec![0, 1])
                        )),
                        Some(f::n::xc(
                            Expression::Primitive(Primitive::Nil),
                            NodeContext::new(1, vec![0, 1])
                        )),
                    ),
                    NodeContext::new(3, vec![0, 1])
                )],
                f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(4, vec![0, 1])
                ),
            )
            .to_fragment(),
            Fragment::Declaration(f::a::view("Foo", vec![3], 4))
        );
    }

    #[test]
    fn module() {
        assert_eq!(
            f::a::mod_(
                "foo",
                ModuleNode(
                    Module::new(
                        vec![Import {
                            source: Source::Root,
                            path: vec![String::from("bar"), String::from("fizz")],
                            aliases: Some(vec![(Target::Module, Some(String::from("Fizz")))]),
                        }],
                        vec![f::n::dc(
                            f::a::const_(
                                "BUZZ",
                                Some(f::n::txc(
                                    TypeExpression::Nil,
                                    NodeContext::new(0, vec![0, 1, 2])
                                )),
                                f::n::xc(
                                    Expression::Primitive(Primitive::Nil),
                                    NodeContext::new(1, vec![0, 1, 2])
                                )
                            ),
                            NodeContext::new(2, vec![0, 1]),
                        )],
                    ),
                    NodeContext::new(3, vec![0, 1]),
                )
            )
            .to_fragment(),
            Fragment::Declaration(f::a::mod_("foo", 3))
        );
    }
}
