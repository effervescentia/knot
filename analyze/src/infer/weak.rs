use crate::{
    context::{FileContext, WeakContext},
    RefKind, Type,
};

#[derive(Clone, Debug, PartialEq)]
pub enum Weak {
    Infer,
    Type(Type<usize>),
    Inherit(usize),
}

pub type WeakRef = (RefKind, Weak);

pub trait ToWeak {
    fn to_weak(&self) -> WeakRef;
}

pub fn infer_types<'a>(file_ctx: FileContext) -> WeakContext {
    let mut ctx = WeakContext::new(file_ctx.fragments);

    ctx.fragments.0.iter().for_each(|(id, (scope, x))| {
        ctx.refs.insert(*id, x.to_weak());

        if let Some(name) = x.to_binding() {
            let entry = ctx
                .bindings
                .0
                .entry((scope.clone(), name.clone()))
                .or_default();

            entry.insert(*id);
        }
    });

    ctx
}

#[cfg(test)]
mod tests {
    use super::Weak;
    use crate::{fragment::Fragment, test::fixture::file_ctx_from, RefKind, Type};
    use lang::{
        ast::{Expression, Primitive, Statement},
        test::fixture as f,
    };
    use std::collections::{BTreeSet, HashMap};

    #[test]
    fn infer_types() {
        let file_ctx = file_ctx_from(vec![(
            1,
            (vec![0], Fragment::Declaration(f::a::type_("MyType", 0))),
        )]);

        let result = super::infer_types(file_ctx);

        assert_eq!(
            result.refs,
            HashMap::from_iter(vec![(1, (RefKind::Type, Weak::Inherit(0)))])
        );

        assert_eq!(
            result.bindings.0,
            HashMap::from_iter(vec![(
                (vec![0], String::from("MyType")),
                (BTreeSet::from_iter(vec![1]))
            )])
        );
    }

    #[test]
    fn type_inheritance() {
        let file_ctx = file_ctx_from(vec![
            (
                0,
                (
                    vec![0, 1],
                    Fragment::Expression(Expression::Primitive(Primitive::Nil)),
                ),
            ),
            (
                1,
                (vec![0], Fragment::Declaration(f::a::const_("FOO", None, 0))),
            ),
            (
                2,
                (
                    vec![0, 2],
                    Fragment::Expression(Expression::Identifier(String::from("FOO"))),
                ),
            ),
            (
                3,
                (vec![0], Fragment::Declaration(f::a::const_("BAR", None, 2))),
            ),
            (
                4,
                (
                    vec![0, 3, 4],
                    Fragment::Expression(Expression::Identifier(String::from("BAR"))),
                ),
            ),
            (
                5,
                (
                    vec![0, 3, 4],
                    Fragment::Statement(Statement::Variable(String::from("fizz"), 4)),
                ),
            ),
            (
                6,
                (
                    vec![0, 3, 4],
                    Fragment::Expression(Expression::Identifier(String::from("fizz"))),
                ),
            ),
            (
                7,
                (vec![0, 3, 4], Fragment::Statement(Statement::Expression(6))),
            ),
            (
                8,
                (
                    vec![0, 3],
                    Fragment::Expression(Expression::Closure(vec![3, 7])),
                ),
            ),
            (
                9,
                (
                    vec![0],
                    Fragment::Declaration(f::a::const_("BUZZ", None, 8)),
                ),
            ),
        ]);

        let result = super::infer_types(file_ctx);

        assert_eq!(
            result.refs,
            HashMap::from_iter(vec![
                (0, (RefKind::Value, Weak::Type(Type::Nil))),
                (1, (RefKind::Value, Weak::Inherit(0))),
                (2, (RefKind::Value, Weak::Infer)),
                (3, (RefKind::Value, Weak::Inherit(2))),
                (4, (RefKind::Value, Weak::Infer)),
                (5, (RefKind::Value, Weak::Type(Type::Nil))),
                (6, (RefKind::Value, Weak::Infer)),
                (7, (RefKind::Value, Weak::Inherit(6))),
                (8, (RefKind::Value, Weak::Inherit(7))),
                (9, (RefKind::Value, Weak::Inherit(8))),
            ])
        );

        assert_eq!(
            result.bindings.0,
            HashMap::from_iter(vec![
                (
                    (vec![0], String::from("FOO")),
                    (BTreeSet::from_iter(vec![1]))
                ),
                (
                    (vec![0], String::from("BAR")),
                    (BTreeSet::from_iter(vec![3]))
                ),
                (
                    (vec![0, 3, 4], String::from("fizz")),
                    (BTreeSet::from_iter(vec![5]))
                ),
                (
                    (vec![0], String::from("BUZZ")),
                    (BTreeSet::from_iter(vec![9]))
                )
            ])
        );
    }

    #[test]
    fn duplicate_bindings() {
        let file_ctx = file_ctx_from(vec![
            (
                1,
                (vec![0], Fragment::Declaration(f::a::type_("MyType", 0))),
            ),
            (
                3,
                (vec![0], Fragment::Declaration(f::a::type_("MyType", 2))),
            ),
        ]);

        let result = super::infer_types(file_ctx);

        assert_eq!(
            result.refs,
            HashMap::from_iter(vec![
                (1, (RefKind::Type, Weak::Inherit(0))),
                (3, (RefKind::Type, Weak::Inherit(2))),
            ])
        );

        assert_eq!(
            result.bindings.0,
            HashMap::from_iter(vec![(
                (vec![0], String::from("MyType")),
                BTreeSet::from_iter(vec![1, 3])
            )])
        );
    }
}
