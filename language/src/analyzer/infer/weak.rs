use crate::analyzer::{context::AnalyzeContext, WeakRef};

pub trait ToWeak {
    fn to_weak(&self) -> WeakRef;
}

pub fn infer_types(ctx: &mut AnalyzeContext) {
    ctx.file
        .borrow()
        .fragments
        .iter()
        .for_each(|(id, (scope, x))| {
            ctx.weak_refs.insert(*id, x.to_weak());

            if let Some(name) = x.to_binding() {
                let entry = ctx
                    .bindings
                    .entry((scope.clone(), name.clone()))
                    .or_default();

                entry.insert(*id);
            }
        })
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::AnalyzeContext, fragment::Fragment, RefKind, Type, Weak},
        parser::{
            expression::{primitive::Primitive, Expression},
            statement::Statement,
        },
        test::fixture as f,
    };
    use std::collections::{BTreeSet, HashMap};

    #[test]
    fn infer_types() {
        let file_ctx = f::f_ctx_from(vec![(
            1,
            (vec![0], Fragment::Declaration(f::a::type_("MyType", 0))),
        )]);
        let mut analyze_ctx = AnalyzeContext::new(&file_ctx);

        super::infer_types(&mut analyze_ctx);

        assert_eq!(
            analyze_ctx.weak_refs,
            HashMap::from_iter(vec![(1, (RefKind::Type, Weak::Inherit(0)))])
        );

        assert_eq!(
            analyze_ctx.bindings,
            HashMap::from_iter(vec![(
                (vec![0], String::from("MyType")),
                (BTreeSet::from_iter(vec![1]))
            )])
        );
    }

    #[test]
    fn type_inheritance() {
        let file_ctx = f::f_ctx_from(vec![
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
                (vec![0, 3, 4], Fragment::Statement(Statement::Effect(6))),
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
        let mut analyze_ctx = AnalyzeContext::new(&file_ctx);

        super::infer_types(&mut analyze_ctx);

        assert_eq!(
            analyze_ctx.weak_refs,
            HashMap::from_iter(vec![
                (0, (RefKind::Value, Weak::Type(Type::Nil))),
                (1, (RefKind::Value, Weak::Inherit(0))),
                (2, (RefKind::Value, Weak::Unknown)),
                (3, (RefKind::Value, Weak::Inherit(2))),
                (4, (RefKind::Value, Weak::Unknown)),
                (5, (RefKind::Value, Weak::Type(Type::Nil))),
                (6, (RefKind::Value, Weak::Unknown)),
                (7, (RefKind::Value, Weak::Inherit(6))),
                (8, (RefKind::Value, Weak::Inherit(7))),
                (9, (RefKind::Value, Weak::Inherit(8))),
            ])
        );

        assert_eq!(
            analyze_ctx.bindings,
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
        let file_ctx = f::f_ctx_from(vec![
            (
                1,
                (vec![0], Fragment::Declaration(f::a::type_("MyType", 0))),
            ),
            (
                3,
                (vec![0], Fragment::Declaration(f::a::type_("MyType", 2))),
            ),
        ]);
        let mut analyze_ctx = AnalyzeContext::new(&file_ctx);

        super::infer_types(&mut analyze_ctx);

        assert_eq!(
            analyze_ctx.weak_refs,
            HashMap::from_iter(vec![
                (1, (RefKind::Type, Weak::Inherit(0))),
                (3, (RefKind::Type, Weak::Inherit(2))),
            ])
        );

        assert_eq!(
            analyze_ctx.bindings,
            HashMap::from_iter(vec![(
                (vec![0], String::from("MyType")),
                BTreeSet::from_iter(vec![1, 3])
            )])
        );
    }
}
