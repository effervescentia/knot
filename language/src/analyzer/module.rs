use super::{
    context::{NodeContext, StrongContext},
    fragment::Fragment,
    infer::{
        strong::ToStrong,
        weak::{ToWeak, Weak, WeakRef},
    },
    register::{Identify, Register, ToFragment},
    RefKind, ScopeContext, Strong,
};
use crate::{
    ast::module::{self, Module, ModuleNode},
    common::position::Decrement,
};
use combine::Stream;
use std::fmt::Debug;

impl<T> ModuleNode<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn id(&self) -> &usize {
        self.1.id()
    }
}

impl<T> Identify<module::NodeValue<T, NodeContext>> for module::NodeValue<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn identify(&self, ctx: &ScopeContext) -> module::NodeValue<T, NodeContext> {
        self.map(&|x| x.register(ctx))
    }
}

impl<T> ToFragment for module::NodeValue<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_fragment<'a>(&'a self) -> Fragment {
        Fragment::Module(self.map(&|x| *x.node().id()))
    }
}

impl<T> Register for ModuleNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    type Node = ModuleNode<T, NodeContext>;
    type Value<C> = module::NodeValue<T, C>;

    fn register(&self, ctx: &ScopeContext) -> ModuleNode<T, NodeContext> {
        let value = self.0.identify(ctx);
        let id = ctx.add_fragment(&value);

        ModuleNode(value, id)
    }
}

impl ToWeak for Module<usize> {
    fn to_weak(&self) -> WeakRef {
        (RefKind::Value, Weak::Infer)
    }
}

impl<'a, T> ToStrong<'a, ModuleNode<T, Strong>> for ModuleNode<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_strong(&self, ctx: &'a StrongContext<'a>) -> ModuleNode<T, Strong> {
        ModuleNode(
            self.0.map(&|x| x.to_strong(ctx)),
            ctx.get_strong_or_fail(self.id()).clone(),
        )
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, fragment::Fragment, register::Register},
        ast::{
            expression::{Expression, Primitive},
            import::{Import, Source, Target},
            module::{Module, ModuleNode},
            type_expression::TypeExpression,
        },
        test::fixture as f,
    };
    use std::collections::BTreeMap;

    #[test]
    fn module() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::n::mr(Module::new(
                vec![Import {
                    source: Source::Root,
                    path: vec![String::from("bar"), String::from("fizz")],
                    aliases: Some(vec![(Target::Module, Some(String::from("Fizz")))]),
                }],
                vec![f::n::d(f::a::const_(
                    "BUZZ",
                    Some(f::n::tx(TypeExpression::Nil)),
                    f::n::x(Expression::Primitive(Primitive::Nil))
                ))],
            ))
            .register(scope),
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
        );

        assert_eq!(
            scope.file.borrow().fragments,
            BTreeMap::from_iter(vec![
                (
                    0,
                    (vec![0, 1], Fragment::TypeExpression(TypeExpression::Nil))
                ),
                (
                    1,
                    (
                        vec![0, 1],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    2,
                    (
                        vec![0],
                        Fragment::Declaration(f::a::const_("BUZZ", Some(0), 1))
                    )
                ),
                (
                    3,
                    (
                        vec![0],
                        Fragment::Module(Module::new(
                            vec![Import {
                                source: Source::Root,
                                path: vec![String::from("bar"), String::from("fizz")],
                                aliases: Some(vec![(Target::Module, Some(String::from("Fizz")))]),
                            }],
                            vec![2],
                        ))
                    )
                ),
            ])
        );
    }
}
