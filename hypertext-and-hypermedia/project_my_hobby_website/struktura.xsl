<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="xml" version="1.0" encoding="UTF-8" indent="yes"/>

    <xsl:template match="/">
        <RaportLekkoatletyczny>
            
            <xsl:element name="{PortalLekkoatletyczny/Naglowek/Autor/Imie}">
                Wersja raportu: <xsl:value-of select="PortalLekkoatletyczny/Naglowek/WersjaProjektu"/>
            </xsl:element>

            <ListaSportowcow>
                <xsl:for-each select="//Zawodnik">
                    
                    <ProfilSportowca>
                        <xsl:attribute name="{../../@typ}">tak</xsl:attribute>

                        <xsl:attribute name="czyAktywny">
                            <xsl:if test="@status='aktywny'">Tak</xsl:if>
                            <xsl:if test="@status!='aktywny'">Nie</xsl:if>
                        </xsl:attribute>

                        <xsl:copy select="@id"/>

                        <DanePersonalne>
                            <Godnosc>
                                <xsl:value-of select="DaneOsobowe/Imie"/>
                                <xsl:text> </xsl:text>
                                <xsl:value-of select="DaneOsobowe/Nazwisko"/>
                            </Godnosc>
                        </DanePersonalne>

                        <Statystyki>
                            <xsl:copy-of select="Osiagniecia/Rekordy"/>
                        </Statystyki>
                        
                    </ProfilSportowca>
                </xsl:for-each>
            </ListaSportowcow>
        </RaportLekkoatletyczny>
    </xsl:template>
</xsl:stylesheet>