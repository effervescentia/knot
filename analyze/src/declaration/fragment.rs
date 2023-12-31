use crate::{context::NodeContext, fragment::Fragment, register::ToFragment};
use lang::ast::{AstNode, DeclarationNodeValue};

impl<R> ToFragment for DeclarationNodeValue<R, NodeContext>
where
    R: Copy,
{
    fn to_fragment(&self) -> Fragment {
        Fragment::Declaration(self.map(
            &|x| *x.node().id(),
            &|x| *x.node().id(),
            &|x| *x.id(),
            &|x| *x.node().id(),
        ))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        context::NodeContext, fragment::Fragment, register::ToFragment, test::fixture as f,
    };
    use kore::str;
    use lang::ast::{
        Expression, Import, ImportSource, ImportTarget, Module, ModuleNode, Parameter, Primitive,
        TypeExpression,
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
                    str!("Bar"),
                    vec![f::n::txc(
                        TypeExpression::Nil,
                        NodeContext::new(0, vec![0, 1])
                    )]
                )]
            )
            .to_fragment(),
            Fragment::Declaration(f::a::enum_("Foo", vec![(str!("Bar"), vec![0])]))
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
                        str!("bar"),
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
                        str!("bar"),
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
            f::a::module(
                "foo",
                ModuleNode(
                    Module::new(
                        vec![f::n::ic(
                            Import {
                                source: ImportSource::Root,
                                path: vec![str!("bar"), str!("fizz")],
                                aliases: Some(vec![(ImportTarget::Module, Some(str!("Fizz")))]),
                            },
                            NodeContext::new(1, vec![0])
                        )],
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
            Fragment::Declaration(f::a::module("foo", 3))
        );
    }
}
