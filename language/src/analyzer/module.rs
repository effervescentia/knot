use super::{context::NodeContext, fragment::Fragment, Analyze, ScopeContext};
use crate::parser::{
    declaration::DeclarationNode,
    module::{Module, ModuleNode},
    position::Decrement,
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

impl<T> Analyze<ModuleNode<T, NodeContext>, Module<usize>> for ModuleNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    type Value<C> = Module<DeclarationNode<T, C>>;

    fn register(self, ctx: &mut ScopeContext) -> ModuleNode<T, NodeContext> {
        let value = Self::identify(self.0, ctx);
        let fragment = Fragment::Module(Self::to_ref(&value));
        let id = ctx.add_fragment(fragment);

        ModuleNode(value, id)
    }

    fn identify(value: Self::Value<()>, ctx: &mut ScopeContext) -> Self::Value<NodeContext> {
        let declarations = value
            .declarations
            .into_iter()
            .map(|x| x.register(ctx))
            .collect::<Vec<_>>();

        Module {
            imports: value.imports,
            declarations,
        }
    }

    fn to_ref<'a>(value: &'a Self::Value<NodeContext>) -> Module<usize> {
        Module {
            imports: value.imports.iter().map(|x| x.clone()).collect::<Vec<_>>(),
            declarations: value
                .declarations
                .iter()
                .map(|x| *x.0.id())
                .collect::<Vec<_>>(),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, fragment::Fragment, Analyze},
        parser::{
            declaration::{
                storage::{Storage, Visibility},
                Declaration,
            },
            expression::{primitive::Primitive, Expression},
            module::{
                import::{Import, Source, Target},
                Module, ModuleNode,
            },
            types::type_expression::TypeExpression,
        },
        test::fixture as f,
    };
    use std::collections::HashMap;

    #[test]
    fn module() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            ModuleNode(
                Module {
                    imports: vec![Import {
                        source: Source::Root,
                        path: vec![String::from("bar"), String::from("fizz")],
                        aliases: Some(vec![(Target::Module, Some(String::from("Fizz")))]),
                    }],
                    declarations: vec![f::dc(
                        Declaration::Constant {
                            name: Storage(Visibility::Public, String::from("BUZZ")),
                            value_type: Some(f::txc(TypeExpression::Nil, ())),
                            value: f::xc(Expression::Primitive(Primitive::Nil), ()),
                        },
                        (),
                    )],
                },
                (),
            )
            .register(scope),
            ModuleNode(
                Module {
                    imports: vec![Import {
                        source: Source::Root,
                        path: vec![String::from("bar"), String::from("fizz")],
                        aliases: Some(vec![(Target::Module, Some(String::from("Fizz")))]),
                    }],
                    declarations: vec![f::dc(
                        Declaration::Constant {
                            name: Storage(Visibility::Public, String::from("BUZZ")),
                            value_type: Some(f::txc(
                                TypeExpression::Nil,
                                NodeContext::new(0, vec![0])
                            )),
                            value: f::xc(
                                Expression::Primitive(Primitive::Nil),
                                NodeContext::new(1, vec![0])
                            ),
                        },
                        NodeContext::new(2, vec![0]),
                    )],
                },
                NodeContext::new(3, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            HashMap::from_iter(vec![
                (0, Fragment::TypeExpression(TypeExpression::Nil)),
                (
                    1,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (
                    2,
                    Fragment::Declaration(Declaration::Constant {
                        name: Storage(Visibility::Public, String::from("BUZZ")),
                        value_type: Some(0),
                        value: 1,
                    })
                ),
                (
                    3,
                    Fragment::Module(Module {
                        imports: vec![Import {
                            source: Source::Root,
                            path: vec![String::from("bar"), String::from("fizz")],
                            aliases: Some(vec![(Target::Module, Some(String::from("Fizz")))]),
                        }],
                        declarations: vec![2],
                    })
                ),
            ])
        );
    }
}
