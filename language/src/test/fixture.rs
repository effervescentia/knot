use crate::{
    declaration::{Declaration, DeclarationRaw},
    expression::{
        ksx::{KSXRaw, KSX},
        Expression, ExpressionRaw,
    },
    module::{Module, ModuleRaw},
    range::Range,
    types::type_expression::{TypeExpression, TypeExpressionRaw},
    CharStream,
};

pub fn xr<'a>(
    x: Expression<ExpressionRaw<CharStream<'a>>, KSXRaw<CharStream<'a>>>,
    (start, end): ((i32, i32), (i32, i32)),
) -> ExpressionRaw<CharStream<'a>> {
    ExpressionRaw(x, Range::chars(start, end))
}

pub fn kxr<'a>(
    x: KSX<ExpressionRaw<CharStream<'a>>, KSXRaw<CharStream<'a>>>,
    (start, end): ((i32, i32), (i32, i32)),
) -> KSXRaw<CharStream<'a>> {
    KSXRaw(x, Range::chars(start, end))
}

pub fn txr<'a>(
    x: TypeExpression<TypeExpressionRaw<CharStream<'a>>>,
    (start, end): ((i32, i32), (i32, i32)),
) -> TypeExpressionRaw<CharStream<'a>> {
    TypeExpressionRaw(x, Range::chars(start, end))
}

pub fn dr<'a>(
    x: Declaration<
        ExpressionRaw<CharStream<'a>>,
        ModuleRaw<CharStream<'a>>,
        TypeExpressionRaw<CharStream<'a>>,
    >,
    (start, end): ((i32, i32), (i32, i32)),
) -> DeclarationRaw<CharStream<'a>> {
    DeclarationRaw(x, Range::chars(start, end))
}

pub fn mr<'a>(x: Module<DeclarationRaw<CharStream<'a>>>) -> ModuleRaw<CharStream<'a>> {
    ModuleRaw(x)
}
