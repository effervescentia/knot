use super::ScopeContext;
use crate::{
    analyzer::{
        context::NodeContext,
        register::{Identify, Register},
    },
    ast::declaration,
    common::position::Decrement,
};
use combine::Stream;
use std::fmt::Debug;

impl<T> Identify<declaration::NodeValue<T, NodeContext>> for declaration::NodeValue<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn identify(&self, ctx: &ScopeContext) -> declaration::NodeValue<T, NodeContext> {
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
        analyzer::{context::NodeContext, register::Identify},
        ast::{
            expression::{Expression, Primitive},
            import::{Import, Source, Target},
            module::{Module, ModuleNode},
            parameter::Parameter,
            type_expression::TypeExpression,
        },
        test::fixture as f,
    };

    #[test]
    fn type_alias() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

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
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::a::enum_(
                "Foo",
                vec![(String::from("Bar"), vec![f::n::tx(TypeExpression::Nil)])]
            )
            .identify(scope),
            f::a::enum_(
                "Foo",
                vec![(
                    String::from("Bar"),
                    vec![f::n::txc(TypeExpression::Nil, NodeContext::new(0, vec![0]))]
                )]
            )
        );
    }

    #[test]
    fn constant() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

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
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::a::func_(
                "foo",
                vec![f::n::p(Parameter::new(
                    String::from("bar"),
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
                        String::from("bar"),
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
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::a::view(
                "Foo",
                vec![f::n::p(Parameter::new(
                    String::from("bar"),
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
                        String::from("bar"),
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
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::a::mod_(
                "foo",
                f::n::mr(Module::new(
                    vec![Import {
                        source: Source::Root,
                        path: vec![String::from("bar"), String::from("fizz")],
                        aliases: Some(vec![(Target::Module, Some(String::from("Fizz")))]),
                    }],
                    vec![f::n::d(f::a::const_(
                        "BUZZ",
                        Some(f::n::tx(TypeExpression::Nil)),
                        f::n::x(Expression::Primitive(Primitive::Nil)),
                    ))],
                ))
            )
            .identify(scope),
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
                                    NodeContext::new(0, vec![0, 1])
                                )),
                                f::n::xc(
                                    Expression::Primitive(Primitive::Nil),
                                    NodeContext::new(1, vec![0, 1])
                                )
                            ),
                            NodeContext::new(2, vec![0]),
                        )],
                    ),
                    NodeContext::new(3, vec![0]),
                )
            )
        );
    }
}
