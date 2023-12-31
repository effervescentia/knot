use super::ScopeContext;
use crate::{
    context::NodeContext,
    register::{Identify, Register},
};
use lang::ast::DeclarationNodeValue;

impl<R> Identify<DeclarationNodeValue<R, NodeContext>> for DeclarationNodeValue<R, ()>
where
    R: Copy,
{
    fn identify(&self, ctx: &ScopeContext) -> DeclarationNodeValue<R, NodeContext> {
        self.map(
            &|x| x.register(ctx),
            &|x| x.register(ctx),
            &|x| x.register(ctx),
            &|x| x.register(ctx),
        )
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        context::NodeContext,
        register::Identify,
        test::fixture as f,
        test::fixture::{file_ctx, scope_ctx},
    };
    use kore::{assert_eq, str};
    use lang::ast::{
        Expression, Import, ImportSource, ImportTarget, Module, ModuleNode, Parameter, Primitive,
        TypeExpression,
    };

    #[test]
    fn type_alias() {
        let file = &file_ctx();
        let scope = &mut scope_ctx(file);

        assert_eq!(
            f::a::type_("Foo", f::n::tx(TypeExpression::Nil)).identify(scope),
            f::a::type_(
                "Foo",
                f::n::txc(TypeExpression::Nil, NodeContext::new(0, vec![0]))
            ),
        );
    }

    #[test]
    fn enumerated() {
        let file = &file_ctx();
        let scope = &mut scope_ctx(file);

        assert_eq!(
            f::a::enum_(
                "Foo",
                vec![(str!("Bar"), vec![f::n::tx(TypeExpression::Nil)])]
            )
            .identify(scope),
            f::a::enum_(
                "Foo",
                vec![(
                    str!("Bar"),
                    vec![f::n::txc(TypeExpression::Nil, NodeContext::new(0, vec![0]))]
                )]
            )
        );
    }

    #[test]
    fn constant() {
        let file = &file_ctx();
        let scope = &mut scope_ctx(file);

        assert_eq!(
            f::a::const_(
                "FOO",
                Some(f::n::tx(TypeExpression::Nil)),
                f::n::x(Expression::Primitive(Primitive::Nil))
            )
            .identify(scope),
            f::a::const_(
                "FOO",
                Some(f::n::txc(TypeExpression::Nil, NodeContext::new(0, vec![0]))),
                f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(1, vec![0])
                )
            )
        );
    }

    #[test]
    fn function() {
        let file = &file_ctx();
        let scope = &mut scope_ctx(file);

        assert_eq!(
            f::a::func_(
                "foo",
                vec![f::n::p(Parameter::new(
                    str!("bar"),
                    Some(f::n::tx(TypeExpression::Nil)),
                    Some(f::n::x(Expression::Primitive(Primitive::Nil))),
                ))],
                Some(f::n::tx(TypeExpression::Nil)),
                f::n::x(Expression::Primitive(Primitive::Nil)),
            )
            .identify(scope),
            f::a::func_(
                "foo",
                vec![f::n::pc(
                    Parameter::new(
                        str!("bar"),
                        Some(f::n::txc(TypeExpression::Nil, NodeContext::new(0, vec![0]))),
                        Some(f::n::xc(
                            Expression::Primitive(Primitive::Nil),
                            NodeContext::new(1, vec![0])
                        )),
                    ),
                    NodeContext::new(2, vec![0])
                )],
                Some(f::n::txc(TypeExpression::Nil, NodeContext::new(3, vec![0]))),
                f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(4, vec![0])
                )
            )
        );
    }

    #[test]
    fn view() {
        let file = &file_ctx();
        let scope = &mut scope_ctx(file);

        assert_eq!(
            f::a::view(
                "Foo",
                vec![f::n::p(Parameter::new(
                    str!("bar"),
                    Some(f::n::tx(TypeExpression::Nil)),
                    Some(f::n::x(Expression::Primitive(Primitive::Nil))),
                ))],
                f::n::x(Expression::Primitive(Primitive::Nil)),
            )
            .identify(scope),
            f::a::view(
                "Foo",
                vec![f::n::pc(
                    Parameter::new(
                        str!("bar"),
                        Some(f::n::txc(TypeExpression::Nil, NodeContext::new(0, vec![0]))),
                        Some(f::n::xc(
                            Expression::Primitive(Primitive::Nil),
                            NodeContext::new(1, vec![0])
                        )),
                    ),
                    NodeContext::new(2, vec![0])
                )],
                f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(3, vec![0])
                ),
            )
        );
    }

    #[test]
    fn module() {
        let file = &file_ctx();
        let scope = &mut scope_ctx(file);

        assert_eq!(
            f::a::module(
                "foo",
                f::n::mr(Module::new(
                    vec![f::n::i(Import {
                        source: ImportSource::Root,
                        path: vec![str!("bar"), str!("fizz")],
                        aliases: Some(vec![(ImportTarget::Module, Some(str!("Fizz")))]),
                    })],
                    vec![f::n::d(f::a::const_(
                        "BUZZ",
                        Some(f::n::tx(TypeExpression::Nil)),
                        f::n::x(Expression::Primitive(Primitive::Nil)),
                    ))],
                ))
            )
            .identify(scope),
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
                            NodeContext::new(0, vec![0])
                        )],
                        vec![f::n::dc(
                            f::a::const_(
                                "BUZZ",
                                Some(f::n::txc(
                                    TypeExpression::Nil,
                                    NodeContext::new(1, vec![0, 2])
                                )),
                                f::n::xc(
                                    Expression::Primitive(Primitive::Nil),
                                    NodeContext::new(2, vec![0, 2])
                                )
                            ),
                            NodeContext::new(3, vec![0]),
                        )],
                    ),
                    NodeContext::new(4, vec![0]),
                )
            )
        );
    }
}
