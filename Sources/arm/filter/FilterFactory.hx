package arm.filter;

class FilterFactory
{


    // static var filterNames: Array<String> = [];

    public static function getFilterNames() : Array<String> {
        // To switch languages
        return [tr("None"), tr("Color Correction")];
    }

    public static function CreateFilter(type: String) : FilterBase {
        if (type == tr("None")) return null;
        if (type == tr("Color Correction")) return new ColorCorrection();

        return null;
    }

    public static function CreateFilterByIndex(index: Int) : FilterBase {
        return CreateFilter(getFilterNames()[index]);
    }

}
