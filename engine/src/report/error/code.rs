pub trait ToCode {
    fn to_code(&self) -> ErrorCode;
}

#[derive(Debug)]
pub struct ErrorCode(pub u16);

impl ErrorCode {
    // 0xx - internal errors

    pub const UNREGISTERED_MODULE: Self = Self(000);

    // 1xx - environment errors

    pub const INVALID_WRITE_TARGET: Self = Self(100);
    pub const INVALID_WRITE_TARGET_NOT_FOUND: Self = Self(101);
    pub const INVALID_WRITE_TARGET_PERMISSION_DENIED: Self = Self(102);
    // [103..109] reserved space for additional writing errors
    pub const INVALID_GLOB: Self = Self(110);
    pub const ROOT_DIRECTORY_NOT_FOUND: Self = Self(111);
    pub const SOURCE_DIRECTORY_NOT_FOUND: Self = Self(112);
    pub const SOURCE_DIRECTORY_NOT_RELATIVE: Self = Self(113);
    pub const ENTRYPOINT_NOT_FOUND: Self = Self(114);
    pub const ENTRYPOINT_NOT_RELATIVE: Self = Self(115);
    pub const CLEANUP_FAILED: Self = Self(116);

    // 2xx - parsing errors

    pub const INVALID_SYNTAX: Self = Self(200);

    // 3xx - linking errors

    pub const MODULE_NOT_FOUND: Self = Self(300);
    pub const IMPORT_CYCLE: Self = Self(301);

    // 4xx - analysis errors

    pub const NOT_INFERRABLE: Self = Self(400);
    pub const NOT_FOUND: Self = Self(401);
    pub const VARIANT_NOT_FOUND: Self = Self(402);
    pub const DECLARATION_NOT_FOUND: Self = Self(403);
    pub const NOT_INDEXABLE: Self = Self(404);
    pub const PROPERTY_NOT_FOUND: Self = Self(405);
    pub const DUPLICATE_PROPERTY: Self = Self(406);
    pub const NOT_SPREADABLE: Self = Self(407);
    pub const UNTYPED_PARAMETER: Self = Self(408);
    pub const DEFAULT_VALUE_REJECTED: Self = Self(409);
    pub const NOT_CALLABLE: Self = Self(410);
    pub const UNEXPECTED_ARGUMENT: Self = Self(411);
    pub const MISSING_ARGUMENT: Self = Self(412);
    pub const ARGUMENT_REJECTED: Self = Self(413);
    pub const NOT_RENDERABLE: Self = Self(414);
    pub const INVALID_COMPONENT: Self = Self(415);
    pub const COMPONENT_TYPO: Self = Self(416);
    pub const INVALID_ATTRIBUTES: Self = Self(417);
    pub const UNEXPECTED_ATTRIBUTE: Self = Self(418);
    pub const MISSING_ATTRIBUTE: Self = Self(419);
    pub const ATTRIBUTE_REJECTED: Self = Self(420);
    pub const BINARY_OPERATION_NOT_SUPPORTED: Self = Self(421);
    pub const UNARY_OPERATION_NOT_SUPPORTED: Self = Self(422);
    pub const UNEXPECTED_KIND: Self = Self(423);

    // 5xx - generation errors
}
