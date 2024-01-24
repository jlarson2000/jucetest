/**
 * Simple model for describing input forms.
 * Converted into Juce components of appropriate types and layout.
 * I find this easier than building forms directly with Juce components.
 */

#pragma once

class Form;

class Field
{
  public:

    enum class Type {
        Int,
        Bool,
        String
    };
    
    Field(const char* name, Type type);
    ~Field();

    const char* getName() {
        return name;
    }

    Type getType() {
        return type;
    }
    
    void setSize(int s) {
        size = s;
    }
    int getSize() {
        return size;
    }
    
    void setMin(int i) {
        min = i;
    }
    int getMin() {
        return min;
    }
    
    void setMax(int i) {
        max = i;
    }
    int getMax() {
        return max;
    }
    
  private:
    
    const char* name = nullptr;
    Type type = Type::Int;
    int size = 0;
    int min = 0;
    int max = 0;
    
};

class Form
{
  public:

    Form();
    ~Form();

    void addField(Field *f);
    juce::OwnedArray<Field>& getFields();
    
  private:

    // start using Juce's type, but look into using std:: at some point, vector?
    juce::OwnedArray<Field> fields;

};

      
