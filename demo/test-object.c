#include "test-object.h"

G_DEFINE_TYPE (TestObject, test_object, G_TYPE_OBJECT);

enum {
    PROP_0,
    PROP_LEN,
    PROP_STR,
    PROP_EQU,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void test_object_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
    TestObject *self = TEST_OBJECT (object);
    switch (property_id) {
        case PROP_LEN:
            self->len = g_value_get_int (value);
            break;
        case PROP_STR:
            g_free (self->str);
            self->str = g_value_dup_string (value);
            break;
        case PROP_EQU:
            self->equal = g_value_get_boolean (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void test_object_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
    TestObject *self = TEST_OBJECT (object);
    switch (property_id) {
        case PROP_LEN:
            g_value_set_int (value, self->len);
            break;
        case PROP_STR:
            g_value_set_string (value, self->str);
            break;
        case PROP_EQU:
            g_value_set_boolean (value, self->equal);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void test_object_dispose (GObject *object)
{
    G_OBJECT_CLASS (test_object_parent_class)->dispose(object);
}

static void test_object_finalize (GObject *object)
{
    TestObject *self = TEST_OBJECT (object);
    g_free(self->str);
    G_OBJECT_CLASS (test_object_parent_class)->finalize(object);
}

static void test_object_class_init(TestObjectClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = test_object_set_property;
    gobject_class->get_property = test_object_get_property;
    gobject_class->dispose = test_object_dispose;
    gobject_class->finalize = test_object_finalize;

    obj_properties[PROP_LEN] = g_param_spec_int ("len", "", "", -1, 256, 0, G_PARAM_READWRITE);
    obj_properties[PROP_STR] = g_param_spec_string ("str", "", "", "Hello world!", G_PARAM_READWRITE);
    obj_properties[PROP_EQU] = g_param_spec_boolean ("equal", "", "", FALSE, G_PARAM_READWRITE);
    g_object_class_install_properties (gobject_class, N_PROPERTIES, obj_properties);
}

static void test_object_init(TestObject *self)
{
    self->len = 0;
    self->str = g_strdup("Hello world!");
    self->equal = FALSE;
}
